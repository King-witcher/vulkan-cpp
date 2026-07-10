#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "panic.h"
#include "allocator.h"

namespace gd
{
#pragma region Buffer
    Buffer::Buffer(Buffer &&other)
        : vmaAllocator{other.vmaAllocator}, allocation{other.allocation}, vkBuffer{other.vkBuffer}
    {
        other.vmaAllocator = nullptr;
    }

    void *Buffer::Map()
    {
        void *ptr;
        vmaMapMemory(vmaAllocator, allocation, &ptr);
        return ptr;
    }

    void Buffer::Unmap()
    {
        vmaUnmapMemory(vmaAllocator, allocation);
    }
#pragma endregion
#pragma region Allocator

    Allocator::Allocator(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device)
    {
        VmaAllocatorCreateInfo info{};
        info.instance = instance;
        info.physicalDevice = physicalDevice;
        info.device = device;
        info.vulkanApiVersion = VK_API_VERSION_1_4;

        if (vmaCreateAllocator(&info, &vmaAllocator))
            Panic("Failed to create VMA allocator");
    }

    Allocator::~Allocator()
    {
        vmaDestroyAllocator(vmaAllocator);
    }

    vk::Result Buffer::MapCopy(std::span<const u8> data)
    {
        auto vkResult = vmaCopyMemoryToAllocation(vmaAllocator, data.data(), allocation, 0, data.size());
        return vk::Result{vkResult};
    }

    vk::ResultValue<Buffer> Allocator::Allocate(vk::BufferCreateInfo bufferInfo)
    {
        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        VkBuffer vkBuffer;
        VmaAllocation allocation;
        auto result =
            vk::Result{vmaCreateBuffer(vmaAllocator, &*bufferInfo, &allocInfo, &vkBuffer, &allocation, nullptr)};

        auto buffer = Buffer{vmaAllocator, allocation, vk::Buffer{vkBuffer}};
        return vk::ResultValue<Buffer>{result, std::move(buffer)};
    }
#pragma endregion
} // namespace gd
