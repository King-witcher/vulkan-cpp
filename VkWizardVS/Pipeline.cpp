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

vkwiz::Pipeline::Pipeline(Device& device, std::string shaderPath)
	: device_(device)
{
	auto shaderCode = readFile(shaderPath);
	createShaderModule(std::move(shaderCode));
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

std::array<vk::PipelineShaderStageCreateInfo, 2> vkwiz::Pipeline::createShaderStages(vk::raii::ShaderModule& shaderModule, const char* vertMain, const char* fragMain)
{
	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
		.stage = vk::ShaderStageFlagBits::eVertex,
		.module = *shaderModule,
		.pName = vertMain,
	};
	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
		.stage = vk::ShaderStageFlagBits::eFragment,
		.module = *shaderModule,
		.pName = fragMain,
	};

	return { vertShaderStageInfo, fragShaderStageInfo };
}
