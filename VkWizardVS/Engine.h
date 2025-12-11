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
		vk::raii::Instance vkInstance = nullptr;
		std::unique_ptr<Window> window = nullptr;
		vk::raii::SurfaceKHR surface = nullptr;
		std::unique_ptr<Device> device_ = nullptr;
		std::unique_ptr<SwapChain> swapChain = nullptr;
		std::unique_ptr<Pipeline> pipeline = nullptr;
		vk::raii::CommandBuffer commandBuffer_ = nullptr;

		vk::raii::Semaphore presentCompleteSemaphore = nullptr;
		vk::raii::Semaphore renderCompleteSemaphore = nullptr;
		vk::raii::Fence drawFence = nullptr;

		vk::raii::Instance createInstance();
		void createCommandBuffer();
		void recordCommandBuffer(vk::Image image, vk::ImageView imageView);
		void createSyncObjects();
		void draw();
	};
}