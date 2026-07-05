#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "device.h"
#include "swapchain.h"

#include <string>

namespace gd
{
    class Pipeline
    {
    public:
        Pipeline(Device &device, gd::Swapchain &swapchain, std::string shaderPath);

        // Must be bound in a renderpass
        vk::raii::Pipeline &VkPipeline() { return vkPipeline; }

    private:
        vk::raii::Pipeline vkPipeline = nullptr;
    };
} // namespace gd
