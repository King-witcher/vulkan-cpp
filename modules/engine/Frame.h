#pragma once

#include "Device.h"

#include <vulkan/vulkan_raii.hpp>

namespace gd
{
    class Frame
    {
        friend class Engine;
        friend class Renderer;

    private:
        Frame(gd::Device &device);

        vk::raii::CommandBuffer commandBuffer = nullptr;
        vk::raii::Semaphore presentReady = nullptr;
        vk::raii::Fence fence = nullptr;
    };
}
