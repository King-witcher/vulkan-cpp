#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "vulkan/vulkan.hpp"

#include "panic.h"
#include "allocator.h"

namespace gd
{
    Buffer::Buffer(Buffer &&other)
        : vmaAllocator{other.vmaAllocator}, allocation{other.allocation},
          vkBuffer{other.vkBuffer}
    {
        other.vmaAllocator = nullptr;
    }

    vk::Result Buffer::Write(const void *pdata, usize size)
    {
        auto vkResult =
            vmaCopyMemoryToAllocation(vmaAllocator, pdata, allocation, 0, size);
        return vk::Result{vkResult};
    }

    Allocator::Allocator(vk::Instance instance,
                         vk::PhysicalDevice physicalDevice, vk::Device device)
    {
        VmaAllocatorCreateInfo info{};
        info.instance = instance;
        info.physicalDevice = physicalDevice;
        info.device = device;
        info.vulkanApiVersion = VK_API_VERSION_1_4;

        if (vmaCreateAllocator(&info, &vmaAllocator))
            Panic("Failed to create VMA allocator");
    }

    vk::ResultValue<Buffer> Allocator::Allocate(vk::BufferCreateInfo bufferInfo)
    {
        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        allocInfo.flags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        VkBuffer vkBuffer;
        VmaAllocation allocation;
        auto result =
            vk::Result{vmaCreateBuffer(vmaAllocator, &*bufferInfo, &allocInfo,
                                       &vkBuffer, &allocation, nullptr)};

        auto buffer = Buffer{vmaAllocator, allocation, vk::Buffer{vkBuffer}};
        return vk::ResultValue<Buffer>{result, std::move(buffer)};
    }

    Allocator::~Allocator()
    {
        vmaDestroyAllocator(vmaAllocator);
    }
} // namespace gd
