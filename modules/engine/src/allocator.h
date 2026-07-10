#pragma once

#include <span>

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

        vk::Result MapCopy(std::span<const u8>);

        template <typename T> inline vk::Result MapCopy(const std::vector<T> &vector)
        {
            std::span<const u8> span{reinterpret_cast<const u8 *>(vector.data()), vector.size() * sizeof(T)};
            return MapCopy(span);
        }

        template <typename T> inline vk::Result MapCopy(const T &data)
        {
            std::span<const u8> span{reinterpret_cast<const u8 *>(&data), sizeof(T)};
            return MapCopy(span);
        }

        void *Map();
        void Unmap();

        vk::Buffer VkBuffer() { return vkBuffer; }

    private:
        Buffer(VmaAllocator allocator, VmaAllocation allocation, vk::Buffer buffer)
            : vmaAllocator{allocator}, allocation{allocation}, vkBuffer{buffer}
        {
        }

        VmaAllocator vmaAllocator;
        VmaAllocation allocation;
        vk::Buffer vkBuffer;
    };

    enum class AllocMode
    {
        HostVisible, // mappable by the CPU (staging, uniforms)
        DeviceLocal, // pure VRAM, no CPU access
    };

    class Allocator
    {
    public:
        Allocator(vk::Instance, vk::PhysicalDevice, vk::Device);
        Allocator(Allocator &) = delete;
        Allocator &operator=(Allocator &) = delete;
        ~Allocator();

        vk::ResultValue<Buffer> Allocate(vk::BufferCreateInfo, AllocMode);

    private:
        VmaAllocator vmaAllocator;
    };
} // namespace gd
