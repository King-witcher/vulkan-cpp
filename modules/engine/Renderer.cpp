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
    renderingInfo.setRenderArea({{0, 0}, extent});
    renderingInfo.setLayerCount(1);
    renderingInfo.setColorAttachments(colorAttachmentInfo);

    frameInFlight.commandBuffer.beginRendering(renderingInfo);

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
            .offset = vk::Offset2D{0, 0},
            .extent = extent});
}

void gd::RenderFrame::endRendering()
{
    frameInFlight.commandBuffer.endRendering();
}

void gd::RenderFrame::transitionRendering()
{
    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = {},
        .srcAccessMask = {},
        .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
        .oldLayout = vk::ImageLayout::eUndefined,
        .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
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
        .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
        .dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
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
    auto &frameInFlight = frames[currentFrame];

    device.waitForFence(frameInFlight.fence);
    device.resetFence(frameInFlight.fence);
    auto &swapchainImage = swapchain.acquireNextImage(frameInFlight.presentReady);

    frameInFlight.commandBuffer.reset();

    frameInFlight.commandBuffer.begin({});
    gd::RenderFrame renderFrame{frameInFlight, swapchainImage};
    renderFrame.transitionRendering();
    renderFrame.beginRendering(swapchain.extent());
    return renderFrame;
}

void gd::Renderer::endFrame(gd::RenderFrame &renderFrame)
{
    auto &frameInFlight = frames[currentFrame];

    renderFrame.endRendering();
    renderFrame.transitionPresentation();
    renderFrame.frameInFlight.commandBuffer.end();

    auto renderReady = renderFrame.swapchainImage.getRenderReadySemaphore();

    // Present to Swapchain
    // TODO: Check SubmitInfo2 and PipelineStageFlagBits2
    vk::SubmitInfo submitInfo;
    vk::PipelineStageFlags waitDstStageMask{vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.setWaitDstStageMask(waitDstStageMask);
    submitInfo.setWaitSemaphores(*renderFrame.frameInFlight.presentReady);
    submitInfo.setCommandBuffers(*renderFrame.frameInFlight.commandBuffer);
    submitInfo.setSignalSemaphores(renderReady);
    device.submitGraphics(submitInfo, frameInFlight.fence);
    swapchain.present(renderFrame.swapchainImage);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
