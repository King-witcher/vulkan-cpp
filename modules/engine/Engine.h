#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vector>

#include "Window.h"
#include "Device.h"
#include "SwapChain.h"
#include "Pipeline.h"
#include "Frame.h"

namespace gd
{
	const u32 MAX_FRAMES_IN_FLIGHT = 2;

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

		std::array<gd::Frame, MAX_FRAMES_IN_FLIGHT> frames = {
			gd::Frame(device_),
			gd::Frame(device_)};

		vk::raii::Instance createInstance() const;
		void recordCommandBuffer(
			vk::raii::CommandBuffer &commandBuffer,
			vk::Image image,
			vk::ImageView imageView,
			vk::raii::Buffer &vertexBuffer,
			u32 vertices);
		void draw(vk::raii::Buffer &vertexBuffer, u32 vertices);
	};
}
