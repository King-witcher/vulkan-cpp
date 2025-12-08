#pragma once

#include "Vulkan.h"
#include "Device.h"
#include "RustTypes.h"

#include <string>
#include <vector>

namespace vkwiz {
	class Pipeline
	{
	public:
		Pipeline(Device& device, std::string shaderPath);

	private:
		Device& device_;
		vk::raii::ShaderModule shaderModule_ = nullptr;

		void createShaderModule(const std::vector<u8> code);
		std::array<vk::PipelineShaderStageCreateInfo, 2> createShaderStages(vk::raii::ShaderModule& shaderModule, const char* vertMain, const char* fragMain);
	};
}