#include <algorithm>

#include "panic.h"
#include "rust_types.h"
#include "swapchain.h"

namespace gd::rhi::vulkan
{
    static vk::SurfaceFormatKHR
    ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &formats)
    {
        for (const auto &format : formats)
        {
            if (format.format == vk::Format::eB8G8R8A8Srgb &&
                format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                return format;
            }
        }
        return formats[0];
    }

    static vk::PresentModeKHR
    ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &)
    {
        // for (const auto& mode : presentModes) {
        //	if (mode == vk::PresentModeKHR::eMailbox) {
        //		return mode;
        //	}
        // }
        return vk::PresentModeKHR::eFifo;
    }

    static vk::Extent2D
    ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities)
    {
        // If width and height are 0xFFFFFFFF, the surface size should be
        // determined by the extent of the swapchain. We are not supporting
        // dynamic surface by now, so let's just discard this scenario.
        if ((capabilities.currentExtent.width |
             capabilities.currentExtent.height) == ~0u)
            Panic("dynamic surface extent is not supported by this engine.");

        return capabilities.currentExtent;
    }

    static u32 ChooseSwapImageCount(const vk::SurfaceCapabilitiesKHR &caps)
    {
        if (!caps.maxImageCount)
            return std::max(caps.minImageCount, 3u);
        return std::clamp(3u, caps.minImageCount, caps.maxImageCount);
    }

    Swapchain::Swapchain(Device &device, vk::SurfaceKHR surface,
                         vk::SwapchainKHR /*oldSwapChain*/)
        : device(device), presentQueue(device.PresentQueue()), surface(surface)
    {
        Recreate();
    }

    SwapchainImage &
    Swapchain::AcquireNextImage(const vk::Semaphore imageAvailable)
    {
        auto [aquireResult, index] =
            vkSwapChain.acquireNextImage(UINT64_MAX, imageAvailable, nullptr);
        switch (aquireResult)
        {
        case vk::Result::eErrorOutOfDateKHR:
        case vk::Result::eSuboptimalKHR:
            Recreate();
            return AcquireNextImage(imageAvailable); // Review it
        case vk::Result::eSuccess:
            break;
        default:
            Panic("failed to acquire swap chain image.");
        }

        return swapchainImages[index];
    }

    void Swapchain::CreateImages(std::vector<vk::Image> images)
    {
        swapchainImages.clear();
        vk::ImageViewCreateInfo viewInfo;
        viewInfo.setViewType(vk::ImageViewType::e2D);
        viewInfo.setFormat(vkImageFormat);
        viewInfo.components.r = vk::ComponentSwizzle::eIdentity;
        viewInfo.components.g = vk::ComponentSwizzle::eIdentity;
        viewInfo.components.b = vk::ComponentSwizzle::eIdentity;
        viewInfo.components.a = vk::ComponentSwizzle::eIdentity;
        viewInfo.subresourceRange.setAspectMask(
            vk::ImageAspectFlagBits::eColor);
        viewInfo.subresourceRange.setBaseMipLevel(0);
        viewInfo.subresourceRange.setLevelCount(1);
        viewInfo.subresourceRange.setBaseArrayLayer(0);
        viewInfo.subresourceRange.setLayerCount(1);

        for (u32 i = 0; i < images.size(); i++)
        {
            viewInfo.setImage(images[i]);
            auto createResult = device.VkDevice().createImageView(viewInfo);
            if (!createResult.has_value())
                Panic("failed to create image view");
            auto semaphore = device.CreateSemaphore();

            swapchainImages.push_back(SwapchainImage(
                i, images[i], std::move(*createResult), std::move(semaphore)));
        }
    }

    void Swapchain::Recreate()
    {
        device.WaitIdle();

        auto swapChainSupport = device.QuerySurfaceSupport(surface);
        auto format = ChooseSwapSurfaceFormat(swapChainSupport.formats);
        auto presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
        extent = ChooseSwapExtent(swapChainSupport.capabilities);
        vkImageFormat = format.format;
        auto minImageCount =
            ChooseSwapImageCount(swapChainSupport.capabilities);

        vk::SwapchainCreateInfoKHR createInfo;
        createInfo.setOldSwapchain(vkSwapChain);
        createInfo.setSurface(surface);
        createInfo.setMinImageCount(minImageCount);
        createInfo.setImageFormat(vkImageFormat);
        createInfo.setImageColorSpace(format.colorSpace);
        createInfo.setImageExtent(extent);
        createInfo.setImageArrayLayers(1);
        createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
        createInfo.setPreTransform(
            swapChainSupport.capabilities.currentTransform);
        createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
        createInfo.setPresentMode(presentMode);
        createInfo.setClipped(vk::True);

        auto createResult = device.VkDevice().createSwapchainKHR(createInfo);
        if (!createResult.has_value())
            Panic("failed to create swapchain.");
        vkSwapChain = std::move(*createResult);

        auto imagesResult = vkSwapChain.getImages();
        if (!imagesResult.has_value())
            Panic("failed to get swapchain images.");

        CreateImages(*imagesResult);
    }

    void Swapchain::Present(SwapchainImage &frame)
    {
        vk::PresentInfoKHR presentInfo;
        auto semaphore = *frame.renderFinished;
        auto swapChain = *vkSwapChain;
        presentInfo.setWaitSemaphores(semaphore);
        presentInfo.setSwapchains(swapChain);
        presentInfo.setImageIndices(frame.index);

        auto result = presentQueue.presentKHR(presentInfo);
        switch (result)
        {
        case vk::Result::eSuccess:
            return;
        case vk::Result::eErrorOutOfDateKHR:
        case vk::Result::eSuboptimalKHR:
            return Recreate();
        default:
            Panic("failed to present swapchain image");
        }
    }
} // namespace gd::rhi::vulkan
