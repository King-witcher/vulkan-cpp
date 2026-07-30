#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "allocator.h"
#include "device.h"
#include "mesh.h"
#include "swapchain.h"
#include "pipeline.h"

namespace gd
{
    // Per-frame transforms uploaded to the shader. Layout must match `UniformBuffer`
    // in shaders/shader.slang (three std140 float4x4; glm::mat4 already matches).
    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    class FrameInFlight
    {
        friend class Renderer;
        friend class RenderPass;

    private:
        FrameInFlight(gd::Device &, gd::Allocator &, vk::raii::CommandPool &, vk::raii::DescriptorPool &,
                      vk::DescriptorSetLayout);

        gd::Buffer ubo;
        vk::raii::CommandBuffer commandBuffer;
        vk::raii::DescriptorSet descriptorSet;
        vk::raii::Semaphore imageAvailable;
        vk::raii::Fence fence;

        static vk::raii::CommandBuffer MakeCommandBuffer(Device &, vk::raii::CommandPool &);
        static gd::Buffer MakeUbo(gd::Allocator &);
        static vk::raii::DescriptorSet MakeDescriptorSet(Device &, vk::raii::DescriptorPool &,
                                                         vk::DescriptorSetLayout);
    };

    class RenderPass
    {
        friend class Renderer;

    public:
        // Linear token: must be consumed by Renderer::SubmitFrame exactly once.
        RenderPass(RenderPass &&);
        RenderPass(const RenderPass &) = delete;
        RenderPass &operator=(const RenderPass &) = delete;
        ~RenderPass();

        void BindPipeline(gd::Pipeline &);
        void BindDescriptorSet(vk::PipelineLayout, vk::DescriptorSet);
        void BindVertexBuffer(gd::Buffer &);
        void BindIndexBuffer(gd::Buffer &);
        void Draw(u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0);
        void DrawIndexed(u32 indexCount, u32 instanceCount = 1, u32 firstIndex = 0, i32 vertexOffset = 0,
                         u32 firstInstance = 0);

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
        bool submitted = false;
    };

    class Renderer
    {
    public:
        Renderer(gd::Device &, gd::Allocator &, gd::Swapchain &);

        gd::RenderPass BeginRenderPass();
        void DrawScene(gd::RenderPass &, std::vector<gd::Mesh> &);
        void SubmitFrame(gd::RenderPass &&);

    private:
        static const u32 MAX_FRAMES_IN_FLIGHT = 2;

        gd::Device *device;
        gd::Allocator *allocator;
        vk::raii::CommandPool commandPool;
        vk::Queue graphicsQueue;
        gd::Swapchain *swapchain;
        gd::Pipeline trianglePipeline;
        // Declared before `frames`: the pool must outlive the descriptor sets it owns
        // (members are destroyed in reverse declaration order).
        vk::raii::DescriptorPool descriptorPool;

        std::array<gd::FrameInFlight, MAX_FRAMES_IN_FLIGHT> frames;
        u32 nextFrame = 0;

        static vk::raii::CommandPool MakeCommandPool(gd::Device &);
        static vk::raii::DescriptorPool MakeDescriptorPool(gd::Device &);
    };
} // namespace gd
