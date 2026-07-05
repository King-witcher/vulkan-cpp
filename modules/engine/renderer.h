#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "frame_in_flight.h"
#include "device.h"
#include "swapchain.h"
#include "pipeline.h"
#include "vulkan/vulkan.hpp"

namespace gd
{
    class RenderFrame
    {
        friend class Renderer;

    public:
        void Draw(vk::raii::Buffer &vertexBuffer, u32 count, gd::Pipeline &pipeline);

    private:
        RenderFrame(
            gd::FrameInFlight &frameInFlight,
            gd::SwapchainImage &swapchainImage)
            : frameInFlight(frameInFlight),
              swapchainImage(swapchainImage) {}

        void BeginRendering(vk::Extent2D extent);
        void EndRendering();
        void TransitionRendering();
        void TransitionPresentation();

        gd::FrameInFlight &frameInFlight;
        gd::SwapchainImage &swapchainImage;
    };

    class Renderer
    {
    public:
        Renderer(gd::Device &device, gd::Swapchain &swapchain) : device(device), swapchain(swapchain) {}

        gd::RenderFrame BeginFrame();
        void EndFrame(gd::RenderFrame &frame);

    private:
        static const u32 MAX_FRAMES_IN_FLIGHT = 2;

        gd::Device &device;
        gd::Swapchain &swapchain;

        std::array<gd::FrameInFlight, MAX_FRAMES_IN_FLIGHT> frames = {
            gd::FrameInFlight(device),
            gd::FrameInFlight(device)};
        u32 nextFrame = 0;
    };
} // namespace gd
