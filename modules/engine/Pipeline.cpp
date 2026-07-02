#include "Pipeline.h"
#include "RustTypes.h"
#include "Mesh.h"

#include <fstream>

using namespace std;

static vector<u8> readFile(const string &filename)
{
	ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file: " + filename);
	}
	vector<u8> buffer(file.tellg());
	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
	return buffer;
}

gd::Pipeline::Pipeline(Device &device, gd::SwapChain &swapchain, std::string shaderPath)
{
	auto &vkDevice = device.vkDevice();
	auto shaderCode = readFile(shaderPath);
	vkShaderModule_ = device.createShaderModule(shaderCode);

	auto bindingDescriptions = Mesh::Vertex::getBindingDescription();
	auto attributeDescriptions = Mesh::Vertex::getAttributeDescriptions();

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
	auto format = swapchain.imageFormat();
	pipelineRenderingInfo.setColorAttachmentFormats(format);

	// Pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	vkPipelineLayout_ = std::move(*vkDevice.createPipelineLayout(pipelineLayoutInfo));

	// Final pipeline create info
	std::vector<vk::PipelineShaderStageCreateInfo> stages = {
		{
			.stage = vk::ShaderStageFlagBits::eVertex,
			.module = *vkShaderModule_,
			.pName = "vertMain",
		},
		{
			.stage = vk::ShaderStageFlagBits::eFragment,
			.module = *vkShaderModule_,
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
	pipelineInfo.setLayout(*vkPipelineLayout_);

	// Sem cache por enquanto
	vkPipeline_ = std::move(*vkDevice.createGraphicsPipeline(nullptr, pipelineInfo));
}
