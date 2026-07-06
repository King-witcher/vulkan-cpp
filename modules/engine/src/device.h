#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "rust_types.h"

namespace gd
{
    struct SurfaceSupport
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    class Device
    {
    public:
        Device(vk::raii::Instance &instance, vk::SurfaceKHR surface);

        SurfaceSupport QuerySurfaceSupport(vk::SurfaceKHR surface);
        void ResetFence(vk::Fence fence);
        vk::Result WaitForFence(vk::Fence fence);
        void SubmitGraphics(vk::SubmitInfo submitInfo, vk::Fence fence);
        void SubmitGraphics2(vk::SubmitInfo2 submitInfo, vk::Fence fence);
        bool Present(vk::PresentInfoKHR &presentInfo);
        std::vector<vk::raii::CommandBuffer>
        AllocateCommandBuffers(u32 count) const;
        vk::raii::Semaphore CreateSemaphore() const;
        vk::raii::Fence CreateFence(bool signaled = true) const;
        vk::raii::ShaderModule
        CreateShaderModule(const std::vector<u8> code) const;

        vk::raii::Device &VkDevice() { return vkDevice; }
        vk::PhysicalDevice PhysicalDevice() { return *vkPhysicalDevice; }
        vk::CommandPool VkCommandPool() { return *vkCommandPool; }
        std::tuple<vk::raii::Buffer, vk::raii::DeviceMemory>
        Allocate(usize size);
        void WaitIdle() const { vkDevice.waitIdle(); }

    private:
        u32 FindMemoryType(u32 typeFilter, vk::MemoryPropertyFlags properties);

        vk::raii::PhysicalDevice vkPhysicalDevice = nullptr;
        vk::raii::Device vkDevice = nullptr;
        vk::raii::Queue vkGraphicsQueue = nullptr;
        vk::raii::Queue vkPresentQueue = nullptr;
        vk::raii::CommandPool vkCommandPool = nullptr;
    };
} // namespace gd
