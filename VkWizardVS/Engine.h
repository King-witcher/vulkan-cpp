#pragma once

#include "Window.h"
#include "Vulkan.h"
#include "Device.h"
#include "SwapChain.h"
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
		std::unique_ptr<Device> device = nullptr;
		std::unique_ptr<SwapChain> swapChain = nullptr;

		vk::raii::Instance createInstance();
	};
}