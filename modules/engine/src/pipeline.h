#pragma once

#include <string>
#include <vulkan/vulkan_raii.hpp>

#include "device.h"

namespace gd
{
    class Pipeline
    {
    public:
        Pipeline(Device &device, vk::Format imageFormat, std::string shaderPath);

        // Must be bound in a renderpass
        vk::Pipeline VkPipeline() { return *vkPipeline; }

    private:
        vk::raii::Pipeline vkPipeline = nullptr;
    };
} // namespace gd
