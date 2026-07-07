#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "device.h"
#include "mesh.h"
#include "swapchain.h"
#include "pipeline.h"
#include "vulkan/vulkan.hpp"

namespace gd
{

    class FrameInFlight
    {
        friend class Renderer;
        friend class RenderFrame;

    private:
        FrameInFlight(gd::Device &device);

        vk::raii::CommandBuffer commandBuffer = nullptr;
        vk::raii::Semaphore imageAvailable = nullptr;
        vk::raii::Fence fence = nullptr;
    };

    class RenderFrame
    {
        friend class Renderer;

    private:
        RenderFrame(gd::FrameInFlight &frameInFlight,
                    gd::SwapchainImage &swapchainImage)
            : frameInFlight(frameInFlight), swapchainImage(swapchainImage)
        {
        }

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
        Renderer(gd::Device &device, gd::Swapchain &swapchain)
            : device(device), graphicsQueue(device.GraphicsQueue()),
              swapchain(swapchain),
              trianglePipeline(device, swapchain.ImageFormat(),
                               "shaders/shader.spv")
        {
        }

        gd::RenderFrame BeginFrame();
        void DrawScene(gd::RenderFrame &, std::vector<gd::Mesh> &);
        void EndFrame(gd::RenderFrame &frame);

    private:
        static const u32 MAX_FRAMES_IN_FLIGHT = 2;

        gd::Device &device;
        vk::Queue graphicsQueue;
        gd::Swapchain &swapchain;
        gd::Pipeline trianglePipeline;

        std::array<gd::FrameInFlight, MAX_FRAMES_IN_FLIGHT> frames = {
            gd::FrameInFlight(device), gd::FrameInFlight(device)};
        u32 nextFrame = 0;
    };
} // namespace gd
