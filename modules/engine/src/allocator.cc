#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "allocator.h"

namespace gd
{
    Allocator::Allocator(vk::raii::Instance &instance,
                         vk::raii::PhysicalDevice &physicalDevice,
                         vk::raii::Device &device)
    {
        VmaAllocatorCreateInfo info{};
        info.instance = *instance;
        info.physicalDevice = *physicalDevice;
        info.device = *device;
        info.vulkanApiVersion = VK_API_VERSION_1_4;

        vmaCreateAllocator(&info, &allocator);
    }

    Allocator::~Allocator()
    {
        vmaDestroyAllocator(allocator);
    }
} // namespace gd
