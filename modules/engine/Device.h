#pragma once

#include "Vulkan.h"
#include "RustTypes.h"

namespace vkwiz
{
	struct SwapchainSurfaceSupportDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	class Device
	{
	public:
		Device(vk::raii::Instance &instance, vk::raii::SurfaceKHR &surface);

		SwapchainSurfaceSupportDetails querySwapchainSupportDetails(vk::raii::SurfaceKHR &surface);
		void resetFence(vk::raii::Fence &fence);
		vk::Result waitForFence(vk::raii::Fence &fence);
		void submitGraphics(vk::SubmitInfo submitInfo, vk::Fence fence);
		bool present(vk::PresentInfoKHR &presentInfo);
		std::vector<vk::raii::CommandBuffer> allocateCommandBuffers(u32 count) const;
		vk::raii::Semaphore createSemaphore() const;
		vk::raii::Fence createFence(bool signaled = true) const;
		vk::raii::ShaderModule createShaderModule(const std::vector<u8> code) const;

		u32 graphicsIndex();
		vk::raii::Device &vkDevice() { return vkDevice_; }
		vk::raii::CommandPool &vkCommandPool() { return vkCommandPool_; }

	private:
		vk::raii::PhysicalDevice vkPhysicalDevice_ = nullptr;
		vk::raii::Device vkDevice_ = nullptr;
		vk::raii::Queue vkGraphicsQueue_ = nullptr;
		vk::raii::Queue vkPresentQueue_ = nullptr;
		vk::raii::CommandPool vkCommandPool_ = nullptr;
		u32 graphicsIndex_ = -1;
	};
}
