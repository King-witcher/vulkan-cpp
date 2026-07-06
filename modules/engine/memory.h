#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "vk_mem_alloc.h"

namespace gd
{
    class Allocator
    {
    public:
        Allocator(vk::raii::Instance &, vk::raii::PhysicalDevice &,
                  vk::raii::Device &);
        Allocator(Allocator &) = delete;
        Allocator &operator=(Allocator &) = delete;
        ~Allocator();

        vk::raii::Buffer CreateBuffer(vk::BufferCreateInfo);

    private:
        VmaAllocator allocator;
    };
} // namespace gd
