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

        vk::raii::Semaphore CreateSemaphore() const;
        vk::raii::Fence CreateFence(bool signaled = true) const;
        void ResetFence(vk::Fence fence) const;
        [[nodiscard]] vk::Result WaitForFence(vk::Fence fence) const;
        [[nodiscard]] vk::Result WaitAndReset(vk::Fence fence) const;

        vk::raii::ShaderModule CreateShaderModule(const std::vector<u8> code) const;
        u32 GraphicsIndex() const { return graphicsIndex; }
        u32 PresentIndex() const { return graphicsIndex; }

        vk::raii::Device &VkDevice() { return vkDevice; }
        vk::PhysicalDevice PhysicalDevice() { return *vkPhysicalDevice; }
        void WaitIdle() const { vkDevice.waitIdle(); }

    private:
        vk::raii::PhysicalDevice vkPhysicalDevice = nullptr;
        vk::raii::Device vkDevice = nullptr;
        u32 graphicsIndex;
        u32 presentIndex;
        vk::raii::CommandPool vkCommandPool = nullptr;

        // Obsolete
        std::tuple<vk::raii::Buffer, vk::raii::DeviceMemory> Allocate(usize size);
        u32 FindMemoryType(u32 typeFilter, vk::MemoryPropertyFlags properties);
    };
} // namespace gd
