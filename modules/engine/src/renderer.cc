#include <vector>

#include "panic.h"

#include "device.h"
#include "mesh.h"
#include "pipeline.h"
#include "renderer.h"
#include "swapchain.h"
#include "vulkan/vulkan.hpp"

namespace gd
{

#pragma region gd::FrameInFlight
    FrameInFlight::FrameInFlight(Device &device, vk::raii::CommandPool &pool)
        : commandBuffer(MakeCommandBuffer(device, pool)), imageAvailable(device.CreateSemaphore()),
          fence(device.CreateFence(true))
    {
    }

    vk::raii::CommandBuffer FrameInFlight::MakeCommandBuffer(Device &device, vk::raii::CommandPool &pool)
    {
        vk::CommandBufferAllocateInfo info;
        info.setCommandPool(pool);
        info.setCommandBufferCount(1);
        info.setLevel(vk::CommandBufferLevel::ePrimary);

        auto result = device.VkDevice().allocateCommandBuffers(info);
        if (!result.has_value())
            Panic("Failed to allocate Command Buffers");
        return std::move((*result)[0]);
    }
#pragma endregion

#pragma region gd::RenderPass
    void gd::RenderPass::BindPipeline(gd::Pipeline &pipeline)
    {
        frameInFlight.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.VkPipeline());
    }

    void gd::RenderPass::BindVertexBuffer(gd::Buffer &buffer)
    {
        frameInFlight.commandBuffer.bindVertexBuffers(0, {buffer.VkBuffer()}, {0});
    }

    void gd::RenderPass::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
    {
        frameInFlight.commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
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
    Renderer::Renderer(gd::Device &device, gd::Swapchain &swapchain)
        : device(&device), commandPool(MakeCommandPool(device)),
          graphicsQueue(device.VkDevice().getQueue(device.GraphicsIndex(), 0)), swapchain(&swapchain),
          trianglePipeline{device, swapchain.ImageFormat(), "shaders/shader.spv"},
          frames{FrameInFlight{device, commandPool}, FrameInFlight{device, commandPool}}
    {
    }

    RenderPass Renderer::BeginRenderPass()
    {
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

    void gd::Renderer::DrawScene(gd::RenderPass &frame, std::vector<gd::Mesh> &scene)
    {
        frame.BindPipeline(trianglePipeline);

        for (auto &mesh : scene)
        {
            frame.BindVertexBuffer(mesh.VertexBuffer());
            frame.Draw(mesh.vertexCount);
        }
    }

    void gd::Renderer::SubmitFrame(gd::RenderPass &renderPass)
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

        swapchain->Present(renderPass.swapchainImage);
    }

    vk::raii::CommandPool Renderer::MakeCommandPool(gd::Device &device)
    {
        vk::CommandPoolCreateInfo info;
        info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        info.setQueueFamilyIndex(device.GraphicsIndex());

        auto result = device.VkDevice().createCommandPool(info);
        if (!result.has_value())
            Panic("Failed to create command pool");

        return std::move(result.value);
    }
#pragma endregion

} // namespace gd
