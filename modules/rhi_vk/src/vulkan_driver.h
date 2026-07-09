#pragma once

#include <unordered_map>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "rhi.h"

#include "allocator.h"
#include "device.h"
#include "pipeline.h"
#include "swapchain.h"

struct SDL_Window;

namespace gd::rhi::vulkan
{
    class VulkanDriver;

    // Grava comandos no command buffer do frame corrente. Recriado (na verdade,
    // só reapontado) a cada BeginFrame.
    class VulkanRenderPass final : public RenderPass
    {
        friend class VulkanDriver;

    public:
        void BindPipeline(PipelineHandle) override;
        void BindVertexBuffer(BufferHandle) override;
        void Draw(u32 vertexCount) override;

    private:
        VulkanDriver *driver = nullptr;
        vk::CommandBuffer commandBuffer = nullptr;
    };

    class VulkanDriver final : public Driver
    {
        friend class VulkanRenderPass;

    public:
        explicit VulkanDriver(SDL_Window *window);
        ~VulkanDriver() override;

        BufferHandle CreateBuffer(const BufferDesc &) override;
        void WriteBuffer(BufferHandle, std::span<const u8> data) override;
        void DestroyBuffer(BufferHandle) override;

        PipelineHandle CreatePipeline(const PipelineDesc &) override;
        void DestroyPipeline(PipelineHandle) override;

        RenderPass &BeginFrame(const ClearColor &) override;
        void EndFrame() override;

        void WaitIdle() override { device.WaitIdle(); }

    private:
        // Um conjunto de objetos de sincronização/gravação por frame em voo.
        struct FrameInFlight
        {
            vk::raii::CommandBuffer commandBuffer = nullptr;
            vk::raii::Semaphore imageAvailable = nullptr;
            vk::raii::Fence fence = nullptr;
        };

        vk::Buffer LookupBuffer(BufferHandle);
        vk::Pipeline LookupPipeline(PipelineHandle);

        void TransitionImage(vk::CommandBuffer, vk::Image,
                             vk::ImageLayout oldLayout,
                             vk::ImageLayout newLayout);

        static constexpr u32 MAX_FRAMES_IN_FLIGHT = 2;

        // Ordem dos membros = ordem de construção; a destruição é reversa, então
        // swapchain/allocator/device morrem antes de surface e instance.
        vk::raii::Context context;
        vk::raii::Instance instance;
        vk::raii::SurfaceKHR surface;
        Device device;
        Allocator allocator;
        Swapchain swapchain;

        std::vector<FrameInFlight> frames;
        u32 nextFrame = 0;

        // Estado do frame corrente, válido entre BeginFrame() e EndFrame().
        VulkanRenderPass currentPass;
        FrameInFlight *currentFrame = nullptr;
        SwapchainImage *currentImage = nullptr;

        std::unordered_map<u32, Buffer> buffers;
        std::unordered_map<u32, PipelineEntry> pipelines;
        u32 nextBufferId = 1;
        u32 nextPipelineId = 1;
    };
} // namespace gd::rhi::vulkan
