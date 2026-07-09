#pragma once

#include <span>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "rust_types.h"

namespace gd::rhi::vulkan
{
    struct SurfaceSupport
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    // Envolve o physical/logical device, as filas e o command pool. Não conhece
    // swapchain nem alocação de memória (VMA cuida disso).
    class Device
    {
    public:
        Device(vk::raii::Instance &instance, vk::SurfaceKHR surface);

        SurfaceSupport QuerySurfaceSupport(vk::SurfaceKHR surface);
        void ResetFence(vk::Fence fence);
        vk::Result WaitForFence(vk::Fence fence);
        std::vector<vk::raii::CommandBuffer>
        AllocateCommandBuffers(u32 count) const;
        vk::raii::Semaphore CreateSemaphore() const;
        vk::raii::Fence CreateFence(bool signaled = true) const;
        vk::raii::ShaderModule CreateShaderModule(std::span<const u8> code) const;

        vk::Queue GraphicsQueue() const { return vkGraphicsQueue; }
        vk::Queue PresentQueue() const { return vkPresentQueue; }
        vk::raii::Device &VkDevice() { return vkDevice; }
        vk::PhysicalDevice PhysicalDevice() { return *vkPhysicalDevice; }
        void WaitIdle() const { vkDevice.waitIdle(); }

    private:
        vk::raii::PhysicalDevice vkPhysicalDevice = nullptr;
        vk::raii::Device vkDevice = nullptr;
        vk::Queue vkGraphicsQueue = nullptr;
        vk::Queue vkPresentQueue = nullptr;
        vk::raii::CommandPool vkCommandPool = nullptr;
    };
} // namespace gd::rhi::vulkan
