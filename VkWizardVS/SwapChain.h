#pragma once
#include "Vulkan.h"
#include "Window.h"
#include "Device.h"

namespace vkwiz {
	class SwapChain
	{
	public:
		SwapChain(Device& device, vk::raii::SurfaceKHR& surface, vk::Extent2D windowExtent);

	private:
		vk::raii::SwapchainKHR swapChain = nullptr;
	};
}
