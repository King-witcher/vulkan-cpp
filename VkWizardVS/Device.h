#pragma once

#include "Vulkan.h"

namespace vkwiz {
	struct SwapchainSurfaceSupportDetails {
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	class Device
	{
	public:
		Device(vk::raii::Instance& instance, vk::raii::SurfaceKHR& surface);
		SwapchainSurfaceSupportDetails querySwapchainSupportDetails(vk::raii::SurfaceKHR& surface, vk::Extent2D windowExtent);
		vk::raii::Device& getDevice();

	private:
		vk::raii::SurfaceKHR& surface;
		vk::raii::PhysicalDevice physicalDevice = nullptr;
		vk::raii::Device device = nullptr;
		vk::raii::Queue graphicsQueue = nullptr;
		vk::raii::Queue presentQueue = nullptr;
	};
}

