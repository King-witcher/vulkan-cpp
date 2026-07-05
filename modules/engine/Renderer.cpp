#include "Renderer.h"
#include "vulkan/vulkan.hpp"

void gd::RenderFrame::draw(vk::raii::Buffer &vertexBuffer, u32 count, gd::Pipeline &pipeline)
{
    frameInFlight.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.vkPipeline());
    // TODO: Experiment bindVertexBuffers2
    frameInFlight.commandBuffer.bindVertexBuffers(0, {vertexBuffer}, {0});
    frameInFlight.commandBuffer.draw(count, 1, 0, 0);
}

void gd::RenderFrame::beginRendering(vk::Extent2D extent)
{
    vk::RenderingAttachmentInfo colorAttachmentInfo;
    colorAttachmentInfo.setImageView(swapchainImage.getImageView());
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

    // Defines the container size inside which the rendered image will be fitted.
    frameInFlight.commandBuffer.setViewport(
        0,
        vk::Viewport(
            0.0f,
            0.0f,
            static_cast<f32>(extent.width),
            static_cast<f32>(extent.height),
            0.0f,
            1.0f));

    frameInFlight.commandBuffer.setScissor(
        0,
        vk::Rect2D{
            .offset = vk::Offset2D{500, 0},
            .extent = extent});
}

void gd::RenderFrame::endRendering()
{
    frameInFlight.commandBuffer.endRendering();
    transitionPresentation();
    frameInFlight.commandBuffer.end();
}

void gd::RenderFrame::transitionRendering()
{
    vk::ImageMemoryBarrier2 barrier = {
        // What the transition should wait before running.
        // Even though there are no commands before the pipeline, sets a dependency on the Color Attachment Output stage.
        // This blocks the transition from happening before the imageAvailable semaphore, which blocks this sage, signals.
        .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        // There are no memory writes to be made available.
        .srcAccessMask = {},
        // What should wait the transition before running.
        .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        // Makes this access visible (invalidate cache)
        .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
        .oldLayout = vk::ImageLayout::eUndefined,
        .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
        // Since we are using the same queue for everything, nothing needs to be transfered.
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapchainImage.getImage(),
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1}};

    vk::DependencyInfo dependencyInfo = {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier};

    frameInFlight.commandBuffer.pipelineBarrier2(dependencyInfo);
}

void gd::RenderFrame::transitionPresentation()
{
    vk::ImageMemoryBarrier2 barrier = {
        // Waits for all Color Attachment Outputs to finish before transitioning back to present optimal layout.
        .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        // Flushes color attachment writes.
        .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
        // There is nothing in this buffer to wait for this barrier to finish. Same as eBottomOfPipe.
        .dstStageMask = {},
        // Nothing to be made available.
        .dstAccessMask = {},
        .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .newLayout = vk::ImageLayout::ePresentSrcKHR,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapchainImage.getImage(),
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1}};

    vk::DependencyInfo dependencyInfo = {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier};

    frameInFlight.commandBuffer.pipelineBarrier2(dependencyInfo);
}

gd::RenderFrame gd::Renderer::beginFrame()
{
    auto &frameInFlight = frames[nextFrame];

    device.waitForFence(frameInFlight.fence);
    device.resetFence(frameInFlight.fence);
    auto &swapchainImage = swapchain.acquireNextImage(frameInFlight.imageAvailable);

    frameInFlight.commandBuffer.reset();

    frameInFlight.commandBuffer.begin({});
    gd::RenderFrame renderFrame{frameInFlight, swapchainImage};
    renderFrame.transitionRendering();
    renderFrame.beginRendering(swapchain.extent());

    nextFrame = (nextFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return renderFrame;
}

void gd::Renderer::endFrame(gd::RenderFrame &renderFrame)
{
    auto &frameInFlight = renderFrame.frameInFlight;
    renderFrame.endRendering();

    auto renderFinished = renderFrame.swapchainImage.getRenderReadySemaphore();

    // Present to Swapchain
    vk::SemaphoreSubmitInfo waitSemaphore;
    waitSemaphore.setSemaphore(renderFrame.frameInFlight.imageAvailable);
    // Trava a escrita na imagem até o acquireNextImage sinalizar. Estágios
    // anteriores (vertex/geometry) podem adiantar enquanto a imagem não chega.
    waitSemaphore.setStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);

    vk::SemaphoreSubmitInfo signalSemaphore;
    signalSemaphore.setSemaphore(renderFinished);
    // Sinaliza só depois de TUDO, incluindo a transitionPresentation() que
    // deixa a imagem em ePresentSrcKHR. Senão o present poderia rodar cedo.
    signalSemaphore.setStageMask(vk::PipelineStageFlagBits2::eAllCommands);

    vk::CommandBufferSubmitInfo commandBufferInfo;
    commandBufferInfo.setCommandBuffer(renderFrame.frameInFlight.commandBuffer);

    // TODO: Check SubmitInfo2 and PipelineStageFlagBits2
    vk::SubmitInfo2 submitInfo;
    submitInfo.setCommandBufferInfos(commandBufferInfo);
    submitInfo.setWaitSemaphoreInfos(waitSemaphore);
    submitInfo.setSignalSemaphoreInfos(signalSemaphore);

    device.submitGraphics2(submitInfo, frameInFlight.fence);
    swapchain.present(renderFrame.swapchainImage);
}
