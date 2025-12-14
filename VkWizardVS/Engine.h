#pragma once

#include "Window.h"
#include "Vulkan.h"
#include "Device.h"
#include "SwapChain.h"
#include "Pipeline.h"

namespace vkwiz {
	class Engine {
	public:
		Engine();

		void run();

	private:
		vk::raii::Context vkContext_;
		Window window_{ "Vulkan Window" };
		vk::raii::Instance vkInstance_ = createInstance();
		vk::raii::SurfaceKHR vkSurface_ = window_.getVulkanSurface(vkInstance_);
		Device device_ = { vkInstance_, vkSurface_ };
		SwapChain swapChain_ = { device_, vkSurface_, window_.getExtent() };
		Pipeline pipeline_ = { device_, swapChain_, "shaders/shader.spv", window_.getExtent() };

		vk::raii::CommandBuffer vkCommandbuffer_ = nullptr;
		vk::raii::Semaphore presentCompleteSemaphore_ = device_.createSemaphore();
		vk::raii::Semaphore renderCompleteSemaphore_ = device_.createSemaphore();
		vk::raii::Fence drawFence_ = device_.createFence();

		vk::raii::Instance createInstance() const;
		void recordCommandBuffer(vk::Image image, vk::ImageView imageView);
		void draw();
	};
}