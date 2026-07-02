#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "Device.h"
#include "RustTypes.h"
#include "SwapChain.h"

#include <string>
#include <vector>

namespace gd
{
	class Pipeline
	{
	public:
		Pipeline(Device &device, gd::Swapchain &swapchain, std::string shaderPath);

		vk::raii::Pipeline &vkPipeline() { return vkPipeline_; }

	private:
		vk::raii::ShaderModule vkShaderModule_ = nullptr;
		vk::raii::PipelineLayout vkPipelineLayout_ = nullptr;
		vk::raii::Pipeline vkPipeline_ = nullptr;
	};
}
