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
        vk::PipelineLayout Layout() { return *vkLayout; }
        vk::DescriptorSetLayout DescriptorSetLayout() { return *descriptorSetLayout; }

    private:
        vk::raii::DescriptorSetLayout descriptorSetLayout;
        vk::raii::PipelineLayout vkLayout;
        vk::raii::Pipeline vkPipeline;

        static vk::raii::DescriptorSetLayout MakeDescriptorSetLayout(Device &);
        static vk::raii::PipelineLayout MakePipelineLayout(Device &, vk::raii::DescriptorSetLayout &);
        static vk::raii::Pipeline MakePipeline(Device &, vk::raii::PipelineLayout &, vk::Format,
                                               const std::string &shaderPath);
    };
} // namespace gd
