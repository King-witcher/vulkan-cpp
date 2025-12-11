#include "SwapChain.h"
#include "RustTypes.h"
using namespace std;

vk::SurfaceFormatKHR chooseSwapSurfaceFormat(vector<vk::SurfaceFormatKHR> formats)
{
	for (const auto& format : formats) {
		if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return format;
		}
	}
	return formats[0];
}

vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> presentModes)
{
	for (const auto& mode : presentModes) {
		if (mode == vk::PresentModeKHR::eMailbox) {
			return mode;
		}
	}
	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR capabilities, vk::Extent2D windowExtent)
{
	if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
		return capabilities.currentExtent;
	}
	// Consider capabilities
	return windowExtent;
}

vkwiz::SwapChain::SwapChain(Device& device, vk::raii::SurfaceKHR& surface, vk::Extent2D windowExtent) : device_(device)
{
	auto swapChainSupport = device.querySwapchainSupportDetails(surface, windowExtent);
	auto format = chooseSwapSurfaceFormat(swapChainSupport.formats);
	auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	extent_ = chooseSwapExtent(swapChainSupport.capabilities, windowExtent);
	// TODO: considerar o surface capabilities
	u32 minImageCount = 3;

	vk::SwapchainCreateInfoKHR swapChainCreateInfo{
		.flags = vk::SwapchainCreateFlagsKHR(),
		.surface = surface,
		.minImageCount = minImageCount,
		.imageFormat = format.format,
		.imageColorSpace = format.colorSpace,
		.imageExtent = extent_,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform = swapChainSupport.capabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE
	};

	swapChain = vk::raii::SwapchainKHR(device.getDevice(), swapChainCreateInfo);
	imageFormat_ = format.format;
	images = swapChain.getImages();

	createImageViews();
}

std::tuple<vk::Image, vk::ImageView> vkwiz::SwapChain::acquireImage(const vk::raii::Semaphore& semaphore, const vk::raii::Fence& fence)
{
	auto [result, index] = swapChain.acquireNextImage(UINT64_MAX, *semaphore, *fence);
	if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
		throw runtime_error("Failed to acquire swap chain image!");
	}
	return { images[index], imageViews[index] };
}

void vkwiz::SwapChain::createImageViews()
{
	auto device = &device_.getDevice();
	auto createInfo = vk::ImageViewCreateInfo{
		.viewType = vk::ImageViewType::e2D,
		.format = imageFormat_,
		.components = {
			.r = vk::ComponentSwizzle::eIdentity,
			.g = vk::ComponentSwizzle::eIdentity,
			.b = vk::ComponentSwizzle::eIdentity,
			.a = vk::ComponentSwizzle::eIdentity,
		},
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};
	imageViews.clear();
	//imageViews = vector<vk::raii::ImageView>(images.size());
	for (auto image : images) {
		createInfo.image = image;
		imageViews.emplace_back(*device, createInfo);
	}
}
