#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "RustTypes.h"

namespace gd
{
	struct SurfaceSupport
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	class Device
	{
	public:
		Device(vk::raii::Instance &instance, vk::raii::SurfaceKHR &surface);

		SurfaceSupport getSurfaceSupport(vk::SurfaceKHR surface);
		void resetFence(vk::raii::Fence &fence);
		vk::Result waitForFence(vk::raii::Fence &fence);
		void submitGraphics(vk::SubmitInfo submitInfo, vk::Fence fence);
		bool present(vk::PresentInfoKHR &presentInfo);
		std::vector<vk::raii::CommandBuffer> allocateCommandBuffers(u32 count) const;
		vk::raii::Semaphore createSemaphore() const;
		vk::raii::Fence createFence(bool signaled = true) const;
		vk::raii::ShaderModule createShaderModule(const std::vector<u8> code) const;

		vk::raii::Device &vkDevice() { return vkDevice_; }
		vk::raii::CommandPool &vkCommandPool() { return vkCommandPool_; }
		std::tuple<vk::raii::Buffer, vk::raii::DeviceMemory> alloc(usize size);
		void waitIdle() const { vkDevice_.waitIdle(); }

	private:
		u32 findMemoryType(u32 typeFilter, vk::MemoryPropertyFlags properties);

		vk::raii::PhysicalDevice vkPhysicalDevice_ = nullptr;
		vk::raii::Device vkDevice_ = nullptr;
		vk::raii::Queue vkGraphicsQueue_ = nullptr;
		vk::raii::Queue vkPresentQueue_ = nullptr;
		vk::raii::CommandPool vkCommandPool_ = nullptr;
	};
}
