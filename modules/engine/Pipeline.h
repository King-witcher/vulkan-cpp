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

		vk::raii::Pipeline& vkPipeline() { return vkPipeline_; }

	private:
		vk::raii::ShaderModule vkShaderModule_ = nullptr;
		vk::raii::PipelineLayout vkPipelineLayout_ = nullptr;
		vk::raii::Pipeline vkPipeline_ = nullptr;
	};
}