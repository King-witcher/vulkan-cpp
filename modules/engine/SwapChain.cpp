#include "SwapChain.h"
#include "RustTypes.h"
#include "Panic.h"

#include <iostream>
#include <algorithm>
using namespace std;

vk::SurfaceFormatKHR chooseSwapSurfaceFormat(vector<vk::SurfaceFormatKHR> formats)
{
	for (const auto &format : formats)
	{
		if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return format;
		}
	}
	return formats[0];
}

vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> presentModes)
{
	// for (const auto& mode : presentModes) {
	//	if (mode == vk::PresentModeKHR::eMailbox) {
	//		return mode;
	//	}
	// }
	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR capabilities)
{
	// If width and height are 0xFFFFFFFF, the surface size should be determined by the extent of the swapchain.
	// We are not supporting dynamic surface by now, so let's just discard this scenario.
	if ((capabilities.currentExtent.width | capabilities.currentExtent.height) == ~0)
		panic("dynamic surface extent is not supported by this engine.");

	return capabilities.currentExtent;
}

u32 chooseSwapImageCount(vk::SurfaceCapabilitiesKHR &capabilities)
{
	if (!capabilities.maxImageCount)
		return std::max(capabilities.minImageCount, 3u);
	return std::clamp(3u, capabilities.minImageCount, capabilities.maxImageCount);
}

vkwiz::SwapChain::SwapChain(Device &device, vk::raii::SurfaceKHR &surface, vk::SwapchainKHR oldSwapChain) : device_(device)
{
	auto swapChainSupport = device.getSurfaceSupport(surface);
	auto format = chooseSwapSurfaceFormat(swapChainSupport.formats);
	auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	extent_ = chooseSwapExtent(swapChainSupport.capabilities);
	vkImageFormat_ = format.format;
	auto minImageCount = chooseSwapImageCount(swapChainSupport.capabilities);

	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.setOldSwapchain(oldSwapChain);
	createInfo.setSurface(surface); // used * before
	createInfo.setMinImageCount(minImageCount);
	createInfo.setImageFormat(vkImageFormat_);
	createInfo.setImageColorSpace(format.colorSpace);
	createInfo.setImageExtent(extent_);
	createInfo.setImageArrayLayers(1); // 1 because we are not doing stereoscopic 3D
	createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
	createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
	createInfo.setPreTransform(swapChainSupport.capabilities.currentTransform);
	createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
	createInfo.setPresentMode(presentMode);
	createInfo.setClipped(vk::True); // Clips pixels that are obscured by other windows. However, this may cause blur effects to bug.
	// swapChainCreateInfo.setOldSwapchain(VK_NULL_HANDLE);

	auto &vkDevice = device.vkDevice();

	auto createResult = vkDevice.createSwapchainKHR(createInfo);
	if (!createResult.has_value())
		panic("failed to create swapchain.");
	vkSwapChain_ = std::move(*createResult);

	auto imagesResult = vkSwapChain_.getImages();
	if (!imagesResult.has_value())
		panic("failed to get swapchain images.");
	vkImages_ = std::move(*imagesResult);

	vkImageViews_ = createImageViews(vkDevice);
}

std::tuple<bool, vk::Image, vk::ImageView, u32> vkwiz::SwapChain::acquireImage(const vk::Semaphore semaphore, const vk::Fence fence)
{
	auto [result, index] = vkSwapChain_.acquireNextImage(UINT64_MAX, semaphore, fence);
	switch (result)
	{
	case vk::Result::eSuccess:
		break;
	case vk::Result::eErrorOutOfDateKHR:
		std::cerr << "swap chain was out of date when acquiring image." << std::endl;
		return {false, {}, {}, static_cast<u32>(0)};
	case vk::Result::eSuboptimalKHR:
		std::cerr << "swap chain was suboptimal when acquiring image." << std::endl;
		return {false, {}, {}, static_cast<u32>(0)};
	default:
		panic("failed to acquire swap chain image.");
	}

	return {true, vkImages_[index], vkImageViews_[index], index};
}

std::vector<vk::raii::ImageView> vkwiz::SwapChain::createImageViews(vk::raii::Device &vkDevice)
{
	std::vector<vk::raii::ImageView> imageViews;
	imageViews.reserve(vkImages_.size());

	vk::ImageViewCreateInfo createInfo;
	createInfo.setViewType(vk::ImageViewType::e2D);
	createInfo.setFormat(vkImageFormat_);
	createInfo.components.r = vk::ComponentSwizzle::eIdentity;
	createInfo.components.g = vk::ComponentSwizzle::eIdentity;
	createInfo.components.b = vk::ComponentSwizzle::eIdentity;
	createInfo.components.a = vk::ComponentSwizzle::eIdentity;
	createInfo.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
	createInfo.subresourceRange.setBaseMipLevel(0);
	createInfo.subresourceRange.setLevelCount(1);
	createInfo.subresourceRange.setBaseArrayLayer(0);
	createInfo.subresourceRange.setLayerCount(1);

	for (int i = 0; i < vkImages_.size(); i++)
	{
		createInfo.setImage(vkImages_[i]);
		auto createResult = vkDevice.createImageView(createInfo);
		if (!createResult.has_value())
			panic("failed to create image view");
		imageViews.push_back(std::move(*createResult));
	}

	return imageViews;
}
