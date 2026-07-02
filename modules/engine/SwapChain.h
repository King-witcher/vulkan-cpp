#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "Device.h"
#include <vector>

namespace gd
{
	struct Frame
	{
	public:
		vk::Semaphore getRenderReadySemaphore() const { return *renderReady; }
		vk::Image getImage() const { return image; }
		vk::ImageView getImageView() const { return *imageView; }

	private:
		Frame(u32 index, vk::Image image, vk::raii::ImageView imageView, vk::raii::Semaphore imageAvailableSemaphore)
			: index(index), image(std::move(image)), imageView(std::move(imageView)), renderReady(std::move(imageAvailableSemaphore)) {}

		u32 index;
		vk::Image image;
		vk::raii::ImageView imageView;
		/** Indicates that the renderer has finished rendering and the image is ready to be presented */
		vk::raii::Semaphore renderReady;

		friend class SwapChain;
	};

	class SwapChain
	{
	public:
		SwapChain(Device &device, vk::raii::SurfaceKHR &surface, vk::SwapchainKHR oldSwapChain = nullptr);

		Frame &acquireNextFrame(const vk::Semaphore semaphore);

		vk::Format imageFormat() const { return vkImageFormat_; }
		vk::Extent2D extent() const { return extent_; }
		vk::raii::SwapchainKHR &vkSwapChain() { return vkSwapChain_; }
		vk::SwapchainKHR operator*() const { return *vkSwapChain_; }
		void present(gd::Frame &frame);

	private:
		Device &device_;
		vk::SurfaceKHR surface_;
		vk::Format vkImageFormat_;
		vk::Extent2D extent_;
		vk::raii::SwapchainKHR vkSwapChain_ = nullptr;
		std::vector<Frame> frames_;

		void createFrames(std::vector<vk::Image> images);

		/** Create or recreate the swapchain. */
		void recreate();
	};
}
