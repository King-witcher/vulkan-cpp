#include <chrono>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include "panic.h"

#include "device.h"
#include "mesh.h"
#include "pipeline.h"
#include "renderer.h"
#include "swapchain.h"
#include "unwrap.h"

namespace gd
{

#pragma region gd::FrameInFlight
    FrameInFlight::FrameInFlight(Device &device, Allocator &allocator, vk::raii::CommandPool &pool,
                                 vk::raii::DescriptorPool &descriptorPool, vk::DescriptorSetLayout layout)
        : ubo(MakeUbo(allocator)), commandBuffer(MakeCommandBuffer(device, pool)),
          descriptorSet(MakeDescriptorSet(device, descriptorPool, layout)), imageAvailable(device.CreateSemaphore()),
          fence(device.CreateFence(true))
    {
        // Point this frame's descriptor set at its own UBO buffer. The buffer is
        // persistent, so this binding is written once and only its contents change
        // per frame — no need to re-update the descriptor set every frame.
        vk::DescriptorBufferInfo bufferInfo;
        bufferInfo.setBuffer(ubo.VkBuffer());
        bufferInfo.setOffset(0);
        bufferInfo.setRange(sizeof(UniformBufferObject));

        vk::WriteDescriptorSet write;
        write.setDstSet(*descriptorSet);
        write.setDstBinding(0);
        write.setDstArrayElement(0);
        write.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        write.setBufferInfo(bufferInfo);

        device.VkDevice().updateDescriptorSets(write, {});
    }

    vk::raii::CommandBuffer FrameInFlight::MakeCommandBuffer(Device &device, vk::raii::CommandPool &pool)
    {
        vk::CommandBufferAllocateInfo info;
        info.setCommandPool(pool);
        info.setCommandBufferCount(1);
        info.setLevel(vk::CommandBufferLevel::ePrimary);

        auto buffers = Unwrap(device.VkDevice().allocateCommandBuffers(info), "Failed to allocate Command Buffers");
        return std::move(buffers[0]);
    }

    gd::Buffer FrameInFlight::MakeUbo(Allocator &allocator)
    {
        vk::BufferCreateInfo info;
        info.setSize(sizeof(UniformBufferObject));
        info.setUsage(vk::BufferUsageFlagBits::eUniformBuffer);
        info.setSharingMode(vk::SharingMode::eExclusive);

        // HostVisible: the CPU rewrites the transforms every frame.
        auto result = allocator.Allocate(info, AllocMode::HostVisible);
        if (result.result != vk::Result::eSuccess)
            Panic("Failed to allocate uniform buffer");
        return std::move(result.value);
    }

    vk::raii::DescriptorSet FrameInFlight::MakeDescriptorSet(Device &device, vk::raii::DescriptorPool &pool,
                                                             vk::DescriptorSetLayout layout)
    {
        vk::DescriptorSetAllocateInfo info;
        info.setDescriptorPool(pool);
        info.setSetLayouts(layout);

        auto sets = Unwrap(device.VkDevice().allocateDescriptorSets(info), "Failed to allocate descriptor set");
        return std::move(sets[0]);
    }
#pragma endregion

#pragma region gd::RenderPass
    RenderPass::RenderPass(RenderPass &&other)
        : frameInFlight(other.frameInFlight), swapchainImage(other.swapchainImage), submitted(other.submitted)
    {
        // The moved-from token is no longer responsible for the frame.
        other.submitted = true;
    }

    RenderPass::~RenderPass()
    {
        // A dropped RenderPass leaves its fence reset but never submitted, so
        // the next wait on that fence would hang forever. Fail loudly instead.
        if (!submitted)
            Panic("RenderPass was dropped without being submitted");
    }

    void gd::RenderPass::BindPipeline(gd::Pipeline &pipeline)
    {
        frameInFlight.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.VkPipeline());
    }

    void gd::RenderPass::BindDescriptorSet(vk::PipelineLayout layout, vk::DescriptorSet set)
    {
        frameInFlight.commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0, {set}, {});
    }

    void gd::RenderPass::BindVertexBuffer(gd::Buffer &buffer)
    {
        frameInFlight.commandBuffer.bindVertexBuffers(0, {buffer.VkBuffer()}, {0});
    }

    void gd::RenderPass::BindIndexBuffer(gd::Buffer &buffer)
    {
        frameInFlight.commandBuffer.bindIndexBuffer(buffer.VkBuffer(), 0, vk::IndexType::eUint32);
    }

    void gd::RenderPass::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
    {
        frameInFlight.commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void gd::RenderPass::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset,
                                     u32 firstInstance)
    {
        frameInFlight.commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void gd::RenderPass::BeginRendering(vk::Extent2D extent)
    {
        vk::RenderingAttachmentInfo colorAttachmentInfo;
        colorAttachmentInfo.setImageView(swapchainImage.ImageView());
        colorAttachmentInfo.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
        colorAttachmentInfo.setLoadOp(vk::AttachmentLoadOp::eClear);
        colorAttachmentInfo.setStoreOp(vk::AttachmentStoreOp::eStore);
        colorAttachmentInfo.setClearValue(vk::ClearColorValue(0.05f, 0.05f, 0.05f, 1.0f));

        vk::RenderingInfo renderingInfo;
        // The rectangle in the image that should be affected by this render pass.
        renderingInfo.setRenderArea({{0, 0}, extent});
        renderingInfo.setLayerCount(1);
        renderingInfo.setColorAttachments(colorAttachmentInfo);

        frameInFlight.commandBuffer.beginRendering(renderingInfo);

        // Defines the container size inside which the rendered image will be
        // fitted.
        frameInFlight.commandBuffer.setViewport(
            0, vk::Viewport(0.0f, 0.0f, static_cast<f32>(extent.width), static_cast<f32>(extent.height), 0.0f, 1.0f));

        frameInFlight.commandBuffer.setScissor(0, vk::Rect2D{.offset = vk::Offset2D{0, 0}, .extent = extent});
    }

    void gd::RenderPass::EndRendering()
    {
        frameInFlight.commandBuffer.endRendering();
        TransitionPresentation();
        frameInFlight.commandBuffer.end();
    }

    void gd::RenderPass::TransitionRendering()
    {
        vk::ImageMemoryBarrier2 barrier = {// What the transition should wait before running.
                                           // Even though there are no commands before the pipeline, sets a
                                           // dependency on the Color Attachment Output stage.
                                           // This blocks the transition from happening before the imageAvailable
                                           // semaphore, which blocks this sage, signals.
                                           .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                                           // There are no memory writes to be made available.
                                           .srcAccessMask = {},
                                           // What should wait the transition before running.
                                           .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                                           // Makes this access visible (invalidate cache)
                                           .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
                                           .oldLayout = vk::ImageLayout::eUndefined,
                                           .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
                                           // Since we are using the same queue for everything, nothing needs to be
                                           // transfered.
                                           .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                           .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                           .image = swapchainImage.Image(),
                                           .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                                                                .baseMipLevel = 0,
                                                                .levelCount = 1,
                                                                .baseArrayLayer = 0,
                                                                .layerCount = 1}};

        vk::DependencyInfo dependencyInfo = {
            .dependencyFlags = {}, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier};

        frameInFlight.commandBuffer.pipelineBarrier2(dependencyInfo);
    }

    void gd::RenderPass::TransitionPresentation()
    {
        vk::ImageMemoryBarrier2 barrier = {// Waits for all Color Attachment Outputs to finish before transitioning
                                           // back to present optimal layout.
                                           .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                                           // Flushes color attachment writes.
                                           .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
                                           // There is nothing in this buffer to wait for this barrier to finish.
                                           // Same as eBottomOfPipe.
                                           .dstStageMask = {},
                                           // Nothing to be made available.
                                           .dstAccessMask = {},
                                           .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
                                           .newLayout = vk::ImageLayout::ePresentSrcKHR,
                                           .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                           .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                           .image = swapchainImage.Image(),
                                           .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                                                                .baseMipLevel = 0,
                                                                .levelCount = 1,
                                                                .baseArrayLayer = 0,
                                                                .layerCount = 1}};

        vk::DependencyInfo dependencyInfo = {
            .dependencyFlags = {}, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier};

        frameInFlight.commandBuffer.pipelineBarrier2(dependencyInfo);
    }
#pragma endregion

#pragma region gd::Renderer
    Renderer::Renderer(gd::Device &device, gd::Allocator &allocator, gd::Swapchain &swapchain)
        : device(&device), allocator(&allocator), commandPool(MakeCommandPool(device)),
          graphicsQueue(device.VkDevice().getQueue(device.GraphicsIndex(), 0)), swapchain(&swapchain),
          trianglePipeline{device, swapchain.ImageFormat(), "shaders/shader.spv"},
          descriptorPool(MakeDescriptorPool(device)),
          frames{FrameInFlight{device, allocator, commandPool, descriptorPool, trianglePipeline.DescriptorSetLayout()},
                 FrameInFlight{device, allocator, commandPool, descriptorPool, trianglePipeline.DescriptorSetLayout()}}
    {
    }

    RenderPass Renderer::BeginRenderPass()
    {
        // Frame boundary: no RenderPass or SwapchainImage references exist and
        // no semaphore is about to be waited on, so recreating is safe here.
        swapchain->RecreateIfNeeded();

        auto &frameInFlight = frames[nextFrame];

        auto result = device->WaitAndReset(frameInFlight.fence);
        if (result != vk::Result::eSuccess)
            Panic("Failed to wait Fence");
        auto &swapchainImage = swapchain->AcquireNextImage(frameInFlight.imageAvailable);

        frameInFlight.commandBuffer.reset();

        frameInFlight.commandBuffer.begin({});
        gd::RenderPass renderPass{frameInFlight, swapchainImage};
        renderPass.TransitionRendering();
        renderPass.BeginRendering(swapchain->Extent());

        nextFrame = (nextFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return renderPass;
    }

    void gd::Renderer::DrawScene(gd::RenderPass &renderPass, std::vector<gd::Mesh> &scene)
    {
        auto &frameInFlight = renderPass.frameInFlight;

        // Seconds since the first frame — drives the spin. BeginRenderPass already
        // waited on this frame's fence, so overwriting its UBO here can't race the GPU.
        static const auto startTime = std::chrono::high_resolution_clock::now();
        auto now = std::chrono::high_resolution_clock::now();
        f32 time = std::chrono::duration<f32>(now - startTime).count();

        auto extent = swapchain->Extent();
        f32 aspect = static_cast<f32>(extent.width) / static_cast<f32>(extent.height);

        UniformBufferObject ubo{};
        // Spin 90°/s around Z (the axis pointing out of the screen).
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);
        // GLM targets OpenGL's Y-up clip space; Vulkan's Y points down. Flipping
        // proj[1][1] fixes it (and is why the pipeline uses CCW front faces).
        ubo.proj[1][1] *= -1.0f;

        frameInFlight.ubo.MapCopy(ubo);

        renderPass.BindPipeline(trianglePipeline);
        renderPass.BindDescriptorSet(trianglePipeline.Layout(), *frameInFlight.descriptorSet);

        for (auto &mesh : scene)
        {
            renderPass.BindVertexBuffer(mesh.VertexBuffer());
            renderPass.BindIndexBuffer(mesh.IndexBuffer());
            renderPass.DrawIndexed(mesh.indexCount);
        }
    }

    void gd::Renderer::SubmitFrame(gd::RenderPass &&renderPass)
    {
        auto &frameInFlight = renderPass.frameInFlight;
        renderPass.EndRendering();

        auto renderFinished = renderPass.swapchainImage.RenderFinishedSemaphore();

        // Present to Swapchain
        vk::SemaphoreSubmitInfo waitSemaphore;
        waitSemaphore.setSemaphore(renderPass.frameInFlight.imageAvailable);
        // Trava a escrita na imagem até o acquireNextImage sinalizar. Estágios
        // anteriores (vertex/geometry) podem adiantar enquanto a imagem não chega.
        waitSemaphore.setStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);

        vk::SemaphoreSubmitInfo signalSemaphore;
        signalSemaphore.setSemaphore(renderFinished);
        // Sinaliza só depois de TUDO, incluindo a transitionPresentation() que
        // deixa a imagem em ePresentSrcKHR. Senão o present poderia rodar cedo.
        signalSemaphore.setStageMask(vk::PipelineStageFlagBits2::eAllCommands);

        vk::CommandBufferSubmitInfo commandBufferInfo;
        commandBufferInfo.setCommandBuffer(renderPass.frameInFlight.commandBuffer);

        // TODO: Check SubmitInfo2 and PipelineStageFlagBits2
        vk::SubmitInfo2 submitInfo;
        submitInfo.setCommandBufferInfos(commandBufferInfo);
        submitInfo.setWaitSemaphoreInfos(waitSemaphore);
        submitInfo.setSignalSemaphoreInfos(signalSemaphore);

        auto submitResult = graphicsQueue.submit2(submitInfo, frameInFlight.fence);
        if (submitResult != vk::Result::eSuccess)
            Panic("failed to submit to graphics queue");
        renderPass.submitted = true;

        swapchain->Present(renderPass.swapchainImage);
    }

    vk::raii::CommandPool Renderer::MakeCommandPool(gd::Device &device)
    {
        vk::CommandPoolCreateInfo info;
        info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        info.setQueueFamilyIndex(device.GraphicsIndex());

        return Unwrap(device.VkDevice().createCommandPool(info), "Failed to create command pool");
    }

    vk::raii::DescriptorPool Renderer::MakeDescriptorPool(gd::Device &device)
    {
        // One uniform-buffer descriptor per frame in flight.
        vk::DescriptorPoolSize poolSize;
        poolSize.setType(vk::DescriptorType::eUniformBuffer);
        poolSize.setDescriptorCount(MAX_FRAMES_IN_FLIGHT);

        vk::DescriptorPoolCreateInfo info;
        // eFreeDescriptorSet: each set is owned by a vk::raii::DescriptorSet, which
        // frees itself on destruction — that requires the pool to allow individual frees.
        info.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
        info.setPoolSizes(poolSize);
        info.setMaxSets(MAX_FRAMES_IN_FLIGHT);

        return Unwrap(device.VkDevice().createDescriptorPool(info), "Failed to create descriptor pool");
    }
#pragma endregion

} // namespace gd
