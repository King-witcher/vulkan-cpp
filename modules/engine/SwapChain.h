#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "Device.h"
#include <vector>

namespace gd
{
	struct Frame
	{
	public:
		vk::Semaphore getSemaphore() const { return *semaphore; }
		vk::Image getImage() const { return image; }
		vk::ImageView getImageView() const { return *imageView; }

	private:
		Frame(u32 index, vk::Image image, vk::raii::ImageView imageView, vk::raii::Semaphore imageAvailableSemaphore)
			: index(index), image(std::move(image)), imageView(std::move(imageView)), semaphore(std::move(imageAvailableSemaphore)) {}

		u32 index;
		vk::Image image;
		vk::raii::ImageView imageView;
		/** Indicates that the monitor has finished reading and the image is ready to be drawn by the GPU */
		vk::raii::Semaphore semaphore;

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
		usize imageCount() const { return frames_.size(); }
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
