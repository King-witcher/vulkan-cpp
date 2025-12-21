#pragma once

#include "Window.h"
#include "Vulkan.h"
#include "Device.h"
#include "SwapChain.h"
#include "Pipeline.h"

#include <vector>

namespace vkwiz
{
	const u32 MAX_FRAMES_IN_FLIGHT = 2;

	class Engine
	{
	public:
		Engine();

		void run();

	private:
		vk::raii::Context vkContext_;
		Window window_{"Vulkan Window"};
		vk::raii::Instance vkInstance_ = createInstance();
		vk::raii::SurfaceKHR vkSurface_ = window_.getVulkanSurface(vkInstance_);
		Device device_ = {vkInstance_, vkSurface_};
		SwapChain swapChain_ = {device_, vkSurface_};
		Pipeline pipeline_ = {device_, swapChain_, "VkWizardVS/shaders/shader.spv", window_.getExtent()};
		u32 frameIndex_ = 0;

		std::vector<vk::raii::CommandBuffer> vkCommandbuffers_ = device_.allocateCommandBuffers(MAX_FRAMES_IN_FLIGHT);
		std::vector<vk::raii::Semaphore> renderFinishedSemaphores_;
		std::vector<vk::raii::Semaphore> presentCompleteSemaphores_;
		std::vector<vk::raii::Fence> inFlightFences_;

		vk::raii::Instance createInstance() const;
		void createSyncObjects();
		void recordCommandBuffer(vk::raii::CommandBuffer &commandBuffer, vk::Image image, vk::ImageView imageView);
		void draw();
	};
}
