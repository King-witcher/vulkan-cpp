#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "rhi.h"
#include "device.h"

namespace gd::rhi::vulkan
{
    // Layout must survive. Needed for push constants/descriptor sets.
    struct PipelineEntry
    {
        vk::raii::PipelineLayout layout = nullptr;
        vk::raii::Pipeline pipeline = nullptr;
    };

    // Builds a pipeline based on the agnostic RHI description.
    // colorFormat is the backbuffer format (dynamic rendering requires it
    // during creation); comes from swapchain.
    PipelineEntry BuildPipeline(Device &device, vk::Format colorFormat,
                                const PipelineDesc &desc);
} // namespace gd::rhi::vulkan
