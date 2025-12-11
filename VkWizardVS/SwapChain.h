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
		vk::Format imageFormat() const { return imageFormat_; }
		vk::Image getImage(u32 index) const { return images[index]; }
		vk::ImageView getImageView(u32 index) const { return imageViews[index]; }
		vk::Extent2D extent() const { return extent_; }
		std::tuple<vk::Image, vk::ImageView> acquireImage(const vk::raii::Semaphore& semaphore, const vk::raii::Fence& fence);

	private:
		Device& device_;
		vk::Format imageFormat_ = vk::Format::eUndefined;
		vk::Extent2D extent_;
		vk::raii::SwapchainKHR swapChain = nullptr;
		vector<vk::Image> images;
		vector<vk::raii::ImageView> imageViews;

		void createImageViews();
	};
}
