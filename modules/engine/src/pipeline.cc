#include <fstream>

#include "panic.h"
#include "rust_types.h"

#include "pipeline.h"
#include "unwrap.h"
#include "vertex.h"

using namespace std;

namespace gd
{

    static vector<u8> ReadFile(const string &filename)
    {
        ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            Panic("Failed to open file: " + filename);
        }
        vector<u8> buffer(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        return buffer;
    }

    gd::Pipeline::Pipeline(Device &device, vk::Format imageFormat, std::string shaderPath)
        : descriptorSetLayout{MakeDescriptorSetLayout(device)},
          vkLayout{MakePipelineLayout(device, descriptorSetLayout)},
          vkPipeline{MakePipeline(device, vkLayout, imageFormat, shaderPath)}
    {
    }

    vk::raii::DescriptorSetLayout Pipeline::MakeDescriptorSetLayout(Device &device)
    {
        auto &vkDevice = device.VkDevice();
        vk::DescriptorSetLayoutBinding uboBinding;
        uboBinding.setBinding(0);
        uboBinding.setDescriptorCount(1); // we can have an array of uniform buffers
        uboBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        uboBinding.setStageFlags(vk::ShaderStageFlagBits::eVertex);

        vk::DescriptorSetLayoutCreateInfo info;
        info.setBindings(uboBinding);

        return Unwrap(vkDevice.createDescriptorSetLayout(info), "Failed to create descriptor set layout");
    }

    vk::raii::PipelineLayout Pipeline::MakePipelineLayout(Device &device,
                                                          vk::raii::DescriptorSetLayout &descriptorSetLayout)
    {

        auto &vkDevice = device.VkDevice();
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        // Attach the UBO descriptor set layout so shaders can read `set = 0, binding = 0`.
        pipelineLayoutInfo.setSetLayouts(*descriptorSetLayout);
        return Unwrap(vkDevice.createPipelineLayout(pipelineLayoutInfo), "Failed to create pipeline layout");
    }

    vk::raii::Pipeline Pipeline::MakePipeline(Device &device, vk::raii::PipelineLayout &layout, vk::Format format,
                                              const std::string &shaderPath)
    {
        auto &vkDevice = device.VkDevice();
        auto shaderCode = ReadFile(shaderPath);
        auto shaderModule = device.CreateShaderModule(shaderCode);

        auto bindingDescriptions = Vertex::BindingDescription();
        auto attributeDescriptions = Vertex::AttributeDescriptions();

        // Fixed functions
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
        vertexInputInfo.setVertexBindingDescriptions(bindingDescriptions);
        vertexInputInfo.setVertexAttributeDescriptions(attributeDescriptions);

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
        inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList);
        // Created dynamically
        // vk::Viewport viewport{
        //	.x = 0.0f,
        //	.y = 0.0f,
        //	.width = static_cast<f32>(extent.width),
        //	.height = static_cast<f32>(extent.height),
        //	.minDepth = 0.0f,
        //	.maxDepth = 1.0f,
        //};
        // vk::Rect2D scissor{
        //	.offset = vk::Offset2D{ 0, 0 },
        //	.extent = extent,
        //};
        vk::PipelineDynamicStateCreateInfo dynamicState;
        std::vector dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor,
        };
        dynamicState.setDynamicStates(dynamicStates);

        vk::PipelineViewportStateCreateInfo viewportState;
        viewportState.setViewportCount(1);
        viewportState.setScissorCount(1);

        vk::PipelineRasterizationStateCreateInfo rasterizer;
        rasterizer.setDepthClampEnable(vk::False);
        rasterizer.setRasterizerDiscardEnable(vk::False);
        rasterizer.setPolygonMode(vk::PolygonMode::eFill);
        rasterizer.setCullMode(vk::CullModeFlagBits::eBack);
        // CCW: the projection flips Y (proj[1][1] *= -1) to match Vulkan's clip space,
        // which reverses the winding the rasterizer sees. Front faces become CCW.
        rasterizer.setFrontFace(vk::FrontFace::eCounterClockwise);
        rasterizer.setDepthBiasEnable(vk::False);
        rasterizer.setDepthBiasSlopeFactor(1.0f);
        rasterizer.setLineWidth(1.0f);

        vk::PipelineMultisampleStateCreateInfo multisampling;
        multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);
        multisampling.setSampleShadingEnable(vk::False);

        vk::PipelineColorBlendAttachmentState colorBlendAttachment;
        colorBlendAttachment.setBlendEnable(vk::False);
        colorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                               vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
        vk::PipelineColorBlendStateCreateInfo colorBlending;
        colorBlending.setLogicOpEnable(vk::False);
        colorBlending.setLogicOp(vk::LogicOp::eCopy);
        colorBlending.setAttachments(colorBlendAttachment);

        // Required for dynamic rendering
        vk::PipelineRenderingCreateInfo pipelineRenderingInfo;
        pipelineRenderingInfo.setColorAttachmentFormats(format);

        // Pipeline layout

        // Final pipeline create info
        std::vector<vk::PipelineShaderStageCreateInfo> stages = {{
                                                                     .stage = vk::ShaderStageFlagBits::eVertex,
                                                                     .module = *shaderModule,
                                                                     .pName = "vertMain",
                                                                 },
                                                                 {
                                                                     .stage = vk::ShaderStageFlagBits::eFragment,
                                                                     .module = *shaderModule,
                                                                     .pName = "fragMain",
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
        pipelineInfo.setLayout(*layout);

        // Sem cache por enquanto
        return Unwrap(vkDevice.createGraphicsPipeline(nullptr, pipelineInfo), "Failed to create graphics pipeline");
    }

} // namespace gd
