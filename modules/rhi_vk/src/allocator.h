#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "vk_mem_alloc.h"

#include "rust_types.h"

namespace gd::rhi::vulkan
{
    class Buffer
    {
        friend class Allocator;

    public:
        ~Buffer()
        {
            if (vmaAllocator)
                vmaDestroyBuffer(vmaAllocator, vkBuffer, allocation);
        }
        Buffer(Buffer &&);
        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;

        vk::Result Write(const void *pdata, usize size);

        vk::Buffer VkBuffer() const { return vkBuffer; }

    private:
        Buffer(VmaAllocator allocator, VmaAllocation allocation,
               vk::Buffer buffer)
            : vmaAllocator{allocator}, allocation{allocation}, vkBuffer{buffer}
        {
        }

        VmaAllocator vmaAllocator;
        VmaAllocation allocation;
        vk::Buffer vkBuffer;
    };

    class Allocator
    {
    public:
        Allocator(vk::Instance, vk::PhysicalDevice, vk::Device);
        Allocator(const Allocator &) = delete;
        Allocator &operator=(const Allocator &) = delete;
        ~Allocator();

        vk::ResultValue<Buffer> Allocate(vk::BufferCreateInfo);

    private:
        VmaAllocator vmaAllocator;
    };
} // namespace gd::rhi::vulkan
