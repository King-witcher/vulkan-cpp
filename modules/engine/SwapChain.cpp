#include "SwapChain.h"
#include "RustTypes.h"
#include "Panic.h"

#include <iostream>
#include <algorithm>
#include "Input.h"
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

gd::Swapchain::Swapchain(Device &device, vk::raii::SurfaceKHR &surface, vk::SwapchainKHR oldSwapChain)
    : device_(device), surface_(surface)
{
    recreate();
}

gd::SwapchainImage &gd::Swapchain::acquireNextImage(const vk::Semaphore imageAvailable)
{
    auto [aquireResult, index] = vkSwapChain_.acquireNextImage(UINT64_MAX, imageAvailable, nullptr);
    switch (aquireResult)
    {
    case vk::Result::eErrorOutOfDateKHR:
    case vk::Result::eSuboptimalKHR:
        recreate();
        return acquireNextImage(imageAvailable); // Review it
    case vk::Result::eSuccess:
        break;
    default:
        panic("failed to acquire swap chain image.");
    }

    return swapchainImages[index];
}

void gd::Swapchain::createImages(std::vector<vk::Image> images)
{
    swapchainImages.clear();
    vk::ImageViewCreateInfo viewInfo;
    viewInfo.setViewType(vk::ImageViewType::e2D);
    viewInfo.setFormat(vkImageFormat_);
    viewInfo.components.r = vk::ComponentSwizzle::eIdentity;
    viewInfo.components.g = vk::ComponentSwizzle::eIdentity;
    viewInfo.components.b = vk::ComponentSwizzle::eIdentity;
    viewInfo.components.a = vk::ComponentSwizzle::eIdentity;
    viewInfo.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
    viewInfo.subresourceRange.setBaseMipLevel(0);
    viewInfo.subresourceRange.setLevelCount(1);
    viewInfo.subresourceRange.setBaseArrayLayer(0);
    viewInfo.subresourceRange.setLayerCount(1);

    for (u32 i = 0; i < images.size(); i++)
    {
        viewInfo.setImage(images[i]);
        auto createResult = device_.vkDevice().createImageView(viewInfo);
        if (!createResult.has_value())
            panic("failed to create image view");
        auto semaphore = device_.createSemaphore();

        swapchainImages.push_back(gd::SwapchainImage(i, images[i], std::move(*createResult), std::move(semaphore)));
    }
}

void gd::Swapchain::recreate()
{
    device_.waitIdle();
    for (; input::minimized(); input::update())
        ;

    auto swapChainSupport = device_.getSurfaceSupport(surface_);
    auto format = chooseSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    extent_ = chooseSwapExtent(swapChainSupport.capabilities);
    vkImageFormat_ = format.format;
    auto minImageCount = chooseSwapImageCount(swapChainSupport.capabilities);

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.setOldSwapchain(vkSwapChain_);
    createInfo.setSurface(surface_); // used * before
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

    auto createResult = device_.vkDevice().createSwapchainKHR(createInfo);
    if (!createResult.has_value())
        panic("failed to create swapchain.");
    vkSwapChain_ = std::move(*createResult);

    auto imagesResult = vkSwapChain_.getImages();
    if (!imagesResult.has_value())
        panic("failed to get swapchain images.");

    createImages(*imagesResult);
}

void gd::Swapchain::present(gd::SwapchainImage &frame)
{
    vk::PresentInfoKHR presentInfo;
    auto semaphore = *frame.renderFinished;
    auto swapChain = *vkSwapChain_;
    presentInfo.setWaitSemaphores(semaphore);
    presentInfo.setSwapchains(swapChain);
    presentInfo.setImageIndices(frame.index);

    if (!device_.present(presentInfo))
        recreate();
}
