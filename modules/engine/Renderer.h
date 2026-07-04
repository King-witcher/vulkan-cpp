#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "FrameInFlight.h"
#include "Device.h"
#include "SwapChain.h"
#include "Pipeline.h"
#include "vulkan/vulkan.hpp"

namespace gd
{
    class RenderFrame
    {
        friend class Renderer;

    public:
        void draw(vk::raii::Buffer &vertexBuffer, u32 count, gd::Pipeline &pipeline);

    private:
        RenderFrame(
            gd::FrameInFlight &frameInFlight,
            gd::SwapchainImage &swapchainImage)
            : frameInFlight(frameInFlight),
              swapchainImage(swapchainImage) {}

        void beginRendering(vk::Extent2D extent);
        void endRendering();
        void transitionRendering();
        void transitionPresentation();

        gd::FrameInFlight &frameInFlight;
        gd::SwapchainImage &swapchainImage;
    };

    class Renderer
    {
    public:
        Renderer(gd::Device &device, gd::Swapchain &swapchain) : device(device), swapchain(swapchain) {}

        gd::RenderFrame beginFrame();
        void endFrame(gd::RenderFrame &frame);

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
