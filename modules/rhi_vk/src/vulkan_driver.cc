#include <iostream>
#include <vector>

#include <vulkan/vulkan.hpp>
#include <SDL3/SDL_vulkan.h>

#include "panic.h"

#include "conversions.h"
#include "vulkan_driver.h"

namespace gd::rhi::vulkan
{
    // -- Construção de instance / surface (SDL) -------------------------------

    static vk::raii::Instance CreateInstance(vk::raii::Context &context,
                                             SDL_Window * /*window*/)
    {
        vk::ApplicationInfo appInfo;
        appInfo.setPApplicationName("VkWizard");
        appInfo.setApplicationVersion(vk::makeVersion(1, 0, 0));
        appInfo.setPEngineName("No Engine");
        appInfo.setEngineVersion(vk::makeVersion(1, 0, 0));
        appInfo.setApiVersion(vk::ApiVersion14);

#ifdef _DEBUG
        auto layers = std::vector<const char *>{"VK_LAYER_KHRONOS_validation"};
        std::cout << "Enabling validation layers..." << std::endl;
#else
        auto layers = std::vector<const char *>{};
#endif

        // A SDL diz de quais extensões de instância precisa para apresentar na
        // janela (VK_KHR_surface + a de plataforma). Independe da janela em si.
        u32 extensionCount = 0;
        auto sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
        std::vector<const char *> extensions(sdlExtensions,
                                             sdlExtensions + extensionCount);

        vk::InstanceCreateInfo createInfo{};
        createInfo.setPApplicationInfo(&appInfo);
        createInfo.setPEnabledLayerNames(layers);
        createInfo.setPEnabledExtensionNames(extensions);

        auto instance = context.createInstance(createInfo);
        if (!instance.has_value())
            Panic("Failed to create Vulkan instance.");
        std::cout << "Vulkan instance created." << std::endl;
        return std::move(*instance);
    }

    static vk::raii::SurfaceKHR CreateSurface(vk::raii::Instance &instance,
                                              SDL_Window *window)
    {
        VkSurfaceKHR surface;
        if (!SDL_Vulkan_CreateSurface(window, *instance, nullptr, &surface))
            Panic("Failed to create Vulkan surface.");
        return vk::raii::SurfaceKHR(instance, surface);
    }

    // -- Ciclo de vida do driver ----------------------------------------------

    VulkanDriver::VulkanDriver(SDL_Window *window)
        : context{}, instance{CreateInstance(context, window)},
          surface{CreateSurface(instance, window)}, device{instance, *surface},
          allocator{*instance, device.PhysicalDevice(), *device.VkDevice()},
          swapchain{device, *surface}
    {
        auto commandBuffers =
            device.AllocateCommandBuffers(MAX_FRAMES_IN_FLIGHT);
        frames.reserve(MAX_FRAMES_IN_FLIGHT);
        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            frames.push_back(FrameInFlight{
                .commandBuffer = std::move(commandBuffers[i]),
                .imageAvailable = device.CreateSemaphore(),
                .fence = device.CreateFence(true),
            });
        }
    }

    VulkanDriver::~VulkanDriver()
    {
        // Garante que nenhum recurso ainda esteja em uso pela GPU antes de os
        // mapas (buffers/pipelines) serem destruídos junto com o objeto.
        device.WaitIdle();
    }

    // -- Recursos
    // --------------------------------------------------------------

    BufferHandle VulkanDriver::CreateBuffer(const BufferDesc &desc)
    {
        vk::BufferCreateInfo bufferInfo;
        bufferInfo.setSize(desc.size);
        bufferInfo.setUsage(ToVk(desc.usage));
        bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

        auto allocated = allocator.Allocate(bufferInfo);
        if (allocated.result != vk::Result::eSuccess)
            Panic("Failed to allocate buffer");

        u32 id = nextBufferId++;
        buffers.emplace(id, std::move(allocated.value));
        return BufferHandle{id};
    }

    void VulkanDriver::WriteBuffer(BufferHandle handle,
                                   std::span<const u8> data)
    {
        auto it = buffers.find(handle.id);
        if (it == buffers.end())
            Panic("WriteBuffer: invalid buffer handle");
        it->second.Write(data.data(), data.size());
    }

    void VulkanDriver::DestroyBuffer(BufferHandle handle)
    {
        buffers.erase(handle.id);
    }

    PipelineHandle VulkanDriver::CreatePipeline(const PipelineDesc &desc)
    {
        auto entry = BuildPipeline(device, swapchain.ImageFormat(), desc);
        u32 id = nextPipelineId++;
        pipelines.emplace(id, std::move(entry));
        return PipelineHandle{id};
    }

    void VulkanDriver::DestroyPipeline(PipelineHandle handle)
    {
        pipelines.erase(handle.id);
    }

    vk::Buffer VulkanDriver::LookupBuffer(BufferHandle handle)
    {
        auto it = buffers.find(handle.id);
        if (it == buffers.end())
            Panic("invalid buffer handle");
        return it->second.VkBuffer();
    }

    vk::Pipeline VulkanDriver::LookupPipeline(PipelineHandle handle)
    {
        auto it = pipelines.find(handle.id);
        if (it == pipelines.end())
            Panic("invalid pipeline handle");
        return *it->second.pipeline;
    }

    // -- Barreiras de layout
    // ---------------------------------------------------

    void VulkanDriver::TransitionImage(vk::CommandBuffer cmd, vk::Image image,
                                       vk::ImageLayout oldLayout,
                                       vk::ImageLayout newLayout)
    {
        vk::ImageMemoryBarrier2 barrier;
        barrier.setOldLayout(oldLayout);
        barrier.setNewLayout(newLayout);
        barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
        barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
        barrier.setImage(image);
        barrier.setSubresourceRange(
            {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

        if (newLayout == vk::ImageLayout::eColorAttachmentOptimal)
        {
            // Undefined -> Color Attachment: bloqueia o estágio de saída de cor
            // até o imageAvailable sinalizar; torna a escrita de cor visível.
            barrier.setSrcStageMask(
                vk::PipelineStageFlagBits2::eColorAttachmentOutput);
            barrier.setSrcAccessMask({});
            barrier.setDstStageMask(
                vk::PipelineStageFlagBits2::eColorAttachmentOutput);
            barrier.setDstAccessMask(
                vk::AccessFlagBits2::eColorAttachmentWrite);
        }
        else
        {
            // Color Attachment -> Present: espera todas as escritas de cor
            // terminarem antes de apresentar.
            barrier.setSrcStageMask(
                vk::PipelineStageFlagBits2::eColorAttachmentOutput);
            barrier.setSrcAccessMask(
                vk::AccessFlagBits2::eColorAttachmentWrite);
            barrier.setDstStageMask({});
            barrier.setDstAccessMask({});
        }

        vk::DependencyInfo dependencyInfo;
        dependencyInfo.setImageMemoryBarriers(barrier);
        cmd.pipelineBarrier2(dependencyInfo);
    }

    RenderPass &VulkanDriver::BeginFrame(const ClearColor &clear)
    {
        auto &frame = frames[nextFrame];

        device.WaitForFence(frame.fence);
        device.ResetFence(frame.fence);
        auto &image = swapchain.AcquireNextImage(frame.imageAvailable);

        auto cmd = *frame.commandBuffer;
        frame.commandBuffer.reset();
        frame.commandBuffer.begin({});

        TransitionImage(cmd, image.Image(), vk::ImageLayout::eUndefined,
                        vk::ImageLayout::eColorAttachmentOptimal);

        auto extent = swapchain.Extent();

        vk::RenderingAttachmentInfo colorAttachment;
        colorAttachment.setImageView(image.ImageView());
        colorAttachment.setImageLayout(
            vk::ImageLayout::eColorAttachmentOptimal);
        colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
        colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
        colorAttachment.setClearValue(
            vk::ClearColorValue(clear.r, clear.g, clear.b, clear.a));

        vk::RenderingInfo renderingInfo;
        renderingInfo.setRenderArea({{0, 0}, extent});
        renderingInfo.setLayerCount(1);
        renderingInfo.setColorAttachments(colorAttachment);

        cmd.beginRendering(renderingInfo);
        cmd.setViewport(
            0, vk::Viewport(0.0f, 0.0f, static_cast<f32>(extent.width),
                            static_cast<f32>(extent.height), 0.0f, 1.0f));
        cmd.setScissor(0, vk::Rect2D{{0, 0}, extent});

        currentFrame = &frame;
        currentImage = &image;
        currentPass.driver = this;
        currentPass.commandBuffer = cmd;

        nextFrame = (nextFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        return currentPass;
    }

    void VulkanDriver::EndFrame()
    {
        auto cmd = currentPass.commandBuffer;

        cmd.endRendering();
        TransitionImage(cmd, currentImage->Image(),
                        vk::ImageLayout::eColorAttachmentOptimal,
                        vk::ImageLayout::ePresentSrcKHR);
        if (cmd.end() != vk::Result::eSuccess)
            Panic("failed to end command buffer");

        auto renderFinished = currentImage->RenderFinishedSemaphore();

        // Trava a escrita na imagem até o acquireNextImage sinalizar. Estágios
        // anteriores (vertex/geometry) podem adiantar enquanto a imagem não
        // chega.
        vk::SemaphoreSubmitInfo waitSemaphore;
        waitSemaphore.setSemaphore(currentFrame->imageAvailable);
        waitSemaphore.setStageMask(
            vk::PipelineStageFlagBits2::eColorAttachmentOutput);

        // Sinaliza só depois de TUDO, incluindo a transição para present.
        vk::SemaphoreSubmitInfo signalSemaphore;
        signalSemaphore.setSemaphore(renderFinished);
        signalSemaphore.setStageMask(vk::PipelineStageFlagBits2::eAllCommands);

        vk::CommandBufferSubmitInfo commandBufferInfo;
        commandBufferInfo.setCommandBuffer(cmd);

        vk::SubmitInfo2 submitInfo;
        submitInfo.setCommandBufferInfos(commandBufferInfo);
        submitInfo.setWaitSemaphoreInfos(waitSemaphore);
        submitInfo.setSignalSemaphoreInfos(signalSemaphore);

        auto submitResult =
            device.GraphicsQueue().submit2(submitInfo, currentFrame->fence);
        if (submitResult != vk::Result::eSuccess)
            Panic("failed to submit to graphics queue");

        swapchain.Present(*currentImage);

        currentFrame = nullptr;
        currentImage = nullptr;
    }

    // -- RenderPass
    // ------------------------------------------------------------

    void VulkanRenderPass::BindPipeline(PipelineHandle handle)
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                   driver->LookupPipeline(handle));
    }

    void VulkanRenderPass::BindVertexBuffer(BufferHandle handle)
    {
        commandBuffer.bindVertexBuffers(0, {driver->LookupBuffer(handle)}, {0});
    }

    void VulkanRenderPass::Draw(u32 vertexCount)
    {
        commandBuffer.draw(vertexCount, 1, 0, 0);
    }

    // -- Fábrica
    // ---------------------------------------------------------------

    std::unique_ptr<Driver> CreateDriver(SDL_Window *window)
    {
        return std::make_unique<VulkanDriver>(window);
    }
} // namespace gd::rhi::vulkan
