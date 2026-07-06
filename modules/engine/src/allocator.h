#pragma once

#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include "vk_mem_alloc.h"
#include "rust_types.h"

namespace gd
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

        vk::Result Write(const void *pdata, usize size);

        template <typename T> vk::Result Write(const std::vector<T> &vector)
        {
            return Write(vector.data(), sizeof(T) * vector.size());
        }

        template <typename T> vk::Result Write(T &data)
        {
            return Write(&data, sizeof(T));
        }

        vk::Buffer VkBuffer() { return vkBuffer; }

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
        Allocator(Allocator &) = delete;
        Allocator &operator=(Allocator &) = delete;
        ~Allocator();

        vk::ResultValue<Buffer> Allocate(vk::BufferCreateInfo);

    private:
        VmaAllocator vmaAllocator;
    };
} // namespace gd
