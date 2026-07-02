#pragma once

#include "Window.h"
#include <vulkan/vulkan_raii.hpp>
#include "Device.h"
#include "SwapChain.h"
#include "Pipeline.h"

#include <vector>

namespace gd
{
	const u32 MAX_FRAMES_IN_FLIGHT = 2;

	class FrameInFlight
	{
		friend class Engine;

	private:
		vk::raii::CommandBuffer commandBuffer;
		vk::raii::Semaphore presentReady;
		vk::raii::Fence inFlightFence;
	};

	class Engine
	{
	public:
		Engine();

		void run();

	private:
		Window window_{"Giuseppe"};
		vk::raii::Context vkContext_;
		vk::raii::Instance vkInstance_ = createInstance();
		vk::raii::SurfaceKHR vkSurface_ = window_.getVulkanSurface(vkInstance_);
		Device device_{vkInstance_, vkSurface_};
		std::unique_ptr<Swapchain> swapChain_ = std::make_unique<Swapchain>(device_, vkSurface_);
		Pipeline pipeline_ = {device_, *swapChain_, "shaders/shader.spv"};
		u32 inFlightIndex = 0;

		std::vector<vk::raii::CommandBuffer> vkCommandbuffers_ = device_.allocateCommandBuffers(MAX_FRAMES_IN_FLIGHT);
		std::vector<vk::raii::Semaphore> presentCompleteSemaphores_;
		std::vector<vk::raii::Fence> inFlightFences_;

		vk::raii::Instance createInstance() const;
		void createSyncObjects();
		void recordCommandBuffer(
			vk::raii::CommandBuffer &commandBuffer,
			vk::Image image,
			vk::ImageView imageView,
			vk::raii::Buffer &vertexBuffer,
			u32 vertices);
		void draw(vk::raii::Buffer &vertexBuffer, u32 vertices);
	};
}
