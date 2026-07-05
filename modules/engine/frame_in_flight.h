#pragma once

#include "device.h"

#include <vulkan/vulkan_raii.hpp>

namespace gd
{
    class FrameInFlight
    {
        friend class Renderer;
        friend class RenderFrame;

    private:
        FrameInFlight(gd::Device &device);

        vk::raii::CommandBuffer commandBuffer = nullptr;
        vk::raii::Semaphore imageAvailable = nullptr;
        vk::raii::Fence fence = nullptr;
    };
} // namespace gd
