#pragma once

#include "Window.h"
#include "Vulkan.h"
#include "Device.h"
#include "SwapChain.h"
#include "Pipeline.h"

#include <memory>

namespace vkwiz {
	class Engine {
	public:
		Engine();

		void run();

	private:
		vk::raii::Context vkContext;
		Window window{ "Vulkan Window" };
		vk::raii::Instance vkInstance = createInstance();
		vk::raii::SurfaceKHR surface = window.getVulkanSurface(vkInstance);
		Device device_ = { vkInstance, surface };
		SwapChain swapChain = { device_, surface, window.getExtent() };
		Pipeline pipeline = { device_, swapChain, "shaders/shader.spv", window.getExtent() };

		vk::raii::CommandBuffer vkCommandbuffer_ = nullptr;
		vk::raii::Semaphore presentCompleteSemaphore = nullptr;
		vk::raii::Semaphore renderCompleteSemaphore = nullptr;
		vk::raii::Fence drawFence = nullptr;

		vk::raii::Instance createInstance() const;
		void recordCommandBuffer(vk::Image image, vk::ImageView imageView);
		void createSyncObjects();
		void draw();
	};
}