#pragma once
#include "Vulkan.h"
#include "Device.h"
#include <vector>

using namespace std;

namespace vkwiz {
	class SwapChain
	{
	public:
		SwapChain(Device& device, vk::raii::SurfaceKHR& surface, vk::Extent2D windowExtent);

		std::tuple<vk::Image, vk::ImageView, u32> acquireImage(const vk::Semaphore semaphore, const vk::Fence fence);

		vk::Image getImage(u32 index) const { return vkImages_[index]; }
		vk::ImageView getImageView(u32 index) const { return vkImageViews_[index]; }

		vk::Format imageFormat() const { return vkImageFormat_; }
		vk::Extent2D extent() const { return extent_; }
		vk::raii::SwapchainKHR& vkSwapChain() { return vkSwapChain_; }
		usize imageCount() const { return vkImages_.size(); }
		vk::SwapchainKHR operator*() const { return *vkSwapChain_; }

	private:
		vk::Format vkImageFormat_;
		vk::Extent2D extent_;
		vk::raii::SwapchainKHR vkSwapChain_ = nullptr;
		vector<vk::Image> vkImages_;
		vector<vk::raii::ImageView> vkImageViews_;

		std::vector<vk::raii::ImageView> createImageViews(vk::raii::Device& vkDevice);
	};
}
