#include <vector>

#include <vulkan/vulkan.hpp>

#include "panic.h"

#include "conversions.h"
#include "pipeline.h"

namespace gd::rhi::vulkan
{
    PipelineEntry BuildPipeline(Device &device, vk::Format colorFormat,
                                const PipelineDesc &desc)
    {
        auto &vkDevice = device.VkDevice();

        auto vertModule = device.CreateShaderModule(desc.vertexShader.spirv);
        auto fragModule = device.CreateShaderModule(desc.fragmentShader.spirv);

        // Vertex input, montado a partir do VertexLayout da RHI.
        vk::VertexInputBindingDescription binding;
        binding.setBinding(0);
        binding.setStride(desc.vertexLayout.stride);
        binding.setInputRate(vk::VertexInputRate::eVertex);

        std::vector<vk::VertexInputAttributeDescription> attributes;
        attributes.reserve(desc.vertexLayout.attributes.size());
        for (const auto &attr : desc.vertexLayout.attributes)
        {
            vk::VertexInputAttributeDescription description;
            description.setLocation(attr.location);
            description.setBinding(0);
            description.setFormat(ToVk(attr.format));
            description.setOffset(attr.offset);
            attributes.push_back(description);
        }

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
        vertexInputInfo.setVertexBindingDescriptions(binding);
        vertexInputInfo.setVertexAttributeDescriptions(attributes);

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
        inputAssembly.setTopology(ToVk(desc.topology));

        std::vector dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor,
        };
        vk::PipelineDynamicStateCreateInfo dynamicState;
        dynamicState.setDynamicStates(dynamicStates);

        vk::PipelineViewportStateCreateInfo viewportState;
        viewportState.setViewportCount(1);
        viewportState.setScissorCount(1);

        vk::PipelineRasterizationStateCreateInfo rasterizer;
        rasterizer.setDepthClampEnable(vk::False);
        rasterizer.setRasterizerDiscardEnable(vk::False);
        rasterizer.setPolygonMode(vk::PolygonMode::eFill);
        rasterizer.setCullMode(ToVk(desc.cullMode));
        rasterizer.setFrontFace(vk::FrontFace::eClockwise);
        rasterizer.setDepthBiasEnable(vk::False);
        rasterizer.setDepthBiasSlopeFactor(1.0f);
        rasterizer.setLineWidth(1.0f);

        vk::PipelineMultisampleStateCreateInfo multisampling;
        multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);
        multisampling.setSampleShadingEnable(vk::False);

        vk::PipelineColorBlendAttachmentState colorBlendAttachment;
        colorBlendAttachment.setBlendEnable(vk::False);
        colorBlendAttachment.setColorWriteMask(
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
        vk::PipelineColorBlendStateCreateInfo colorBlending;
        colorBlending.setLogicOpEnable(vk::False);
        colorBlending.setLogicOp(vk::LogicOp::eCopy);
        colorBlending.setAttachments(colorBlendAttachment);

        // Required for dynamic rendering
        vk::PipelineRenderingCreateInfo pipelineRenderingInfo;
        pipelineRenderingInfo.setColorAttachmentFormats(colorFormat);

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        auto pipelineLayoutResult =
            vkDevice.createPipelineLayout(pipelineLayoutInfo);
        if (!pipelineLayoutResult.has_value())
            Panic("Failed to create pipeline layout");
        auto pipelineLayout = std::move(*pipelineLayoutResult);

        std::vector<vk::PipelineShaderStageCreateInfo> stages = {
            {
                .stage = vk::ShaderStageFlagBits::eVertex,
                .module = *vertModule,
                .pName = desc.vertexShader.entryPoint,
            },
            {
                .stage = vk::ShaderStageFlagBits::eFragment,
                .module = *fragModule,
                .pName = desc.fragmentShader.entryPoint,
            }};

        vk::GraphicsPipelineCreateInfo pipelineInfo;
        pipelineInfo.setPNext(&pipelineRenderingInfo);
        pipelineInfo.setStages(stages);
        pipelineInfo.setPVertexInputState(&vertexInputInfo);
        pipelineInfo.setPInputAssemblyState(&inputAssembly);
        pipelineInfo.setPViewportState(&viewportState);
        pipelineInfo.setPRasterizationState(&rasterizer);
        pipelineInfo.setPMultisampleState(&multisampling);
        pipelineInfo.setPColorBlendState(&colorBlending);
        pipelineInfo.setPDynamicState(&dynamicState);
        pipelineInfo.setLayout(*pipelineLayout);

        auto pipelineResult =
            vkDevice.createGraphicsPipeline(nullptr, pipelineInfo);
        if (!pipelineResult.has_value())
            Panic("Failed to create graphics pipeline");

        return PipelineEntry{
            .layout = std::move(pipelineLayout),
            .pipeline = std::move(*pipelineResult),
        };
    }
} // namespace gd::rhi::vulkan
