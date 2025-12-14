#pragma once

#include "Vulkan.h"
#include "RustTypes.h"

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
		vk::raii::Device& vkDevice();
		u32 graphicsIndex();
		void resetFence(vk::Fence fence);
		vk::Result waitForFence(vk::Fence fence);
		void submitGraphics(vk::SubmitInfo submitInfo, vk::Fence fence);
		void present(vk::PresentInfoKHR presentInfo);
		std::vector<vk::raii::CommandBuffer> allocateCommandBuffers(u32 count) const;
		vk::raii::CommandPool& vkCommandPool() { return vkCommandPool_; }

	private:
		vk::raii::SurfaceKHR& vkSurface;
		vk::raii::PhysicalDevice vkPhysicalDevice_ = nullptr;
		vk::raii::Device vkDevice_ = nullptr;
		vk::raii::Queue vkGraphicsQueue_ = nullptr;
		vk::raii::Queue vkPresentQueue_ = nullptr;
		vk::raii::CommandPool vkCommandPool_ = nullptr;
		u32 graphicsIndex_ = -1;
	};
}

