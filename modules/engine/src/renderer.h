#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "device.h"
#include "mesh.h"
#include "swapchain.h"
#include "pipeline.h"

namespace gd
{

    class FrameInFlight
    {
        friend class Renderer;
        friend class RenderPass;

    private:
        FrameInFlight(gd::Device &, vk::raii::CommandPool &);

        vk::raii::CommandBuffer commandBuffer;
        vk::raii::Semaphore imageAvailable;
        vk::raii::Fence fence;

        static vk::raii::CommandBuffer MakeCommandBuffer(Device &, vk::raii::CommandPool &);
    };

    class RenderPass
    {
        friend class Renderer;

    public:
        void BindPipeline(gd::Pipeline &);
        void BindVertexBuffer(gd::Buffer &);
        void Draw(u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0);

    private:
        RenderPass(gd::FrameInFlight &frameInFlight, gd::SwapchainImage &swapchainImage)
            : frameInFlight(frameInFlight), swapchainImage(swapchainImage)
        {
        }

        void BeginRendering(vk::Extent2D);
        void EndRendering();
        void TransitionRendering();
        void TransitionPresentation();

        gd::FrameInFlight &frameInFlight;
        gd::SwapchainImage &swapchainImage;
    };

    class Renderer
    {
    public:
        Renderer(gd::Device &, gd::Swapchain &);

        gd::RenderPass BeginRenderPass();
        void DrawScene(gd::RenderPass &, std::vector<gd::Mesh> &);
        void SubmitFrame(gd::RenderPass &);

    private:
        static const u32 MAX_FRAMES_IN_FLIGHT = 2;

        gd::Device *device;
        vk::raii::CommandPool commandPool;
        vk::Queue graphicsQueue;
        gd::Swapchain *swapchain;
        gd::Pipeline trianglePipeline;

        std::array<gd::FrameInFlight, MAX_FRAMES_IN_FLIGHT> frames;
        u32 nextFrame = 0;

        static vk::raii::CommandPool MakeCommandPool(gd::Device &);
    };
} // namespace gd
