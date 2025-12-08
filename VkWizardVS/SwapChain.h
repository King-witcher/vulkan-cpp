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

	private:
		Device& device_;
		vk::Format imageFormat = vk::Format::eUndefined;
		vk::Extent2D extent;
		vk::raii::SwapchainKHR swapChain = nullptr;
		vector<vk::Image> images;
		vector<vk::ImageView> imageViews;

		void createImageViews();
	};
}
