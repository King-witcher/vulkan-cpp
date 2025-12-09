#include "Pipeline.h"
#include "RustTypes.h"

#include <fstream>

using namespace std;

static vector<u8> readFile(const string& filename) {
	ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file: " + filename);
	}
	vector<u8> buffer(file.tellg());
	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
	file.close();
	return buffer;
}

vkwiz::Pipeline::Pipeline(vkwiz::Device& device, std::string shaderPath, vk::Extent2D extent)
	: device_(device)
{
	auto logicalDevice = &device_.getDevice();
	auto shaderCode = readFile(shaderPath);
	createShaderModule(std::move(shaderCode));

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
		.stage = vk::ShaderStageFlagBits::eVertex,
		.module = *shaderModule_,
		.pName = "vertMain",
	};

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
		.stage = vk::ShaderStageFlagBits::eFragment,
		.module = *shaderModule_,
		.pName = "fragMain",
	};

	// Fixed functions
	std::array dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
	};

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
		.topology = vk::PrimitiveTopology::eTriangleList,
	};
	// Created dynamically
	//vk::Viewport viewport{
	//	.x = 0.0f,
	//	.y = 0.0f,
	//	.width = static_cast<f32>(extent.width),
	//	.height = static_cast<f32>(extent.height),
	//	.minDepth = 0.0f,
	//	.maxDepth = 1.0f,
	//};
	//vk::Rect2D scissor{
	//	.offset = vk::Offset2D{ 0, 0 },
	//	.extent = extent,
	//};
	vk::PipelineDynamicStateCreateInfo dynamicState{
		.dynamicStateCount = static_cast<u32>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data(),
	};
	vk::PipelineViewportStateCreateInfo viewportState{
		.viewportCount = 1,
		.scissorCount = 1,
	};
	vk::PipelineRasterizationStateCreateInfo rasterizer{
		.depthClampEnable = vk::False,
		.rasterizerDiscardEnable = vk::False,
		.polygonMode = vk::PolygonMode::eLine,
		.cullMode = vk::CullModeFlagBits::eBack,
		.frontFace = vk::FrontFace::eClockwise,
		.depthBiasEnable = vk::False,
		.depthBiasSlopeFactor = 1.0f,
		.lineWidth = 1.0f,
	};
	vk::PipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable = vk::False,
		.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
						 vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
	};
	vk::PipelineColorBlendStateCreateInfo colorBlending{
		.logicOpEnable = vk::False,
		.logicOp = vk::LogicOp::eCopy,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
	};
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayout_ = vk::raii::PipelineLayout{ *logicalDevice, pipelineLayoutInfo };

}

void vkwiz::Pipeline::createShaderModule(const std::vector<u8> code)
{
	auto device = &device_.getDevice();
	vk::ShaderModuleCreateInfo createInfo{
		.codeSize = code.size(),
		.pCode = reinterpret_cast<const u32*>(code.data()),
	};
	shaderModule_ = { *device, createInfo };
}
