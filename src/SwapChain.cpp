#include "SwapChain.h"
#include "RustTypes.h"
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
		throw new runtime_error("Dynamic surface extent is not supported by this engine.");

	return capabilities.currentExtent;
}

vkwiz::SwapChain::SwapChain(Device &device, vk::raii::SurfaceKHR &surface) : device_(device)
{
	auto swapChainSupport = device.querySwapchainSupportDetails(surface);
	auto format = chooseSwapSurfaceFormat(swapChainSupport.formats);
	auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	extent_ = chooseSwapExtent(swapChainSupport.capabilities);
	vkImageFormat_ = format.format;
	// TODO: considerar o surface capabilities
	u32 minImageCount = swapChainSupport.capabilities.minImageCount + 1;

	vk::SwapchainCreateInfoKHR swapChainCreateInfo;
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
	vkSwapChain_ = vkDevice.createSwapchainKHR(swapChainCreateInfo);

	vkImages_ = vkSwapChain_.getImages();
	vkImageViews_ = createImageViews(vkDevice);
}

std::tuple<vk::Image, vk::ImageView, u32> vkwiz::SwapChain::acquireImage(const vk::Semaphore semaphore, const vk::Fence fence)
{
	auto [result, index] = vkSwapChain_.acquireNextImage(UINT64_MAX, semaphore, fence);
	if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
	{
		throw runtime_error("Failed to acquire swap chain image!");
	}
	return {vkImages_[index], vkImageViews_[index], index};
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
		imageViews.emplace_back(vkDevice, createInfo);
	}

	return imageViews;
}
