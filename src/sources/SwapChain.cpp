#include "SwapChain.h"
#include "RustTypes.h"
#include "Panic.h"
#include <iostream>
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
	if (capabilities.currentExtent.width == std::numeric_limits<u32>::max())
		panic("dynamic surface extent is not supported by this engine.");

	return capabilities.currentExtent;
}

vkwiz::SwapChain::SwapChain(Device &device, vk::raii::SurfaceKHR &surface, vk::SwapchainKHR oldSwapChain) : device_(device)
{
	auto swapChainSupport = device.querySwapchainSupportDetails(surface);
	auto format = chooseSwapSurfaceFormat(swapChainSupport.formats);
	auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	extent_ = chooseSwapExtent(swapChainSupport.capabilities);
	vkImageFormat_ = format.format;
	// TODO: considerar o surface capabilities
	u32 minImageCount = swapChainSupport.capabilities.minImageCount + 1;

	vk::SwapchainCreateInfoKHR swapChainCreateInfo;
	swapChainCreateInfo.setOldSwapchain(oldSwapChain);
	swapChainCreateInfo.setSurface(*surface);
	swapChainCreateInfo.setMinImageCount(minImageCount);
	swapChainCreateInfo.setImageFormat(vkImageFormat_);
	swapChainCreateInfo.setImageColorSpace(format.colorSpace);
	swapChainCreateInfo.setImageExtent(extent_);
	swapChainCreateInfo.setImageArrayLayers(1);
	swapChainCreateInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
	swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
	swapChainCreateInfo.setPreTransform(swapChainSupport.capabilities.currentTransform);
	swapChainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
	swapChainCreateInfo.setPresentMode(presentMode);
	swapChainCreateInfo.setClipped(vk::True);
	// swapChainCreateInfo.setOldSwapchain(VK_NULL_HANDLE);

	auto &vkDevice = device.vkDevice();
	auto result = vkDevice.createSwapchainKHR(swapChainCreateInfo);
	vkSwapChain_ = std::move(*result);

	vkImages_ = std::move(*vkSwapChain_.getImages());
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

	for (auto image : vkImages_)
	{
		createInfo.setImage(image);
		auto view = std::move(*vkDevice.createImageView(createInfo));
		imageViews.push_back(std::move(view));
	}

	return imageViews;
}
