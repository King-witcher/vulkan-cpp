#pragma once

#include "Vulkan.h"
#include "Device.h"
#include "RustTypes.h"
#include "SwapChain.h"

#include <string>
#include <vector>

namespace vkwiz {
	class Pipeline
	{
	public:
		Pipeline(Device& device, vkwiz::SwapChain& swapchain, std::string shaderPath, vk::Extent2D extent);
		vk::raii::Pipeline& getPipeline() { return pipeline_; }

	private:
		Device& device_;
		vk::raii::ShaderModule shaderModule_ = nullptr;
		vk::raii::PipelineLayout pipelineLayout_ = nullptr;
		vk::raii::Pipeline pipeline_ = nullptr;

		void createShaderModule(const std::vector<u8> code);
	};
}