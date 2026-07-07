#pragma once

#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "rust_types.h"
#include "device.h"

namespace gd
{
    struct SwapchainImage
    {
    public:
        vk::Semaphore RenderFinishedSemaphore() const
        {
            return *renderFinished;
        }
        vk::Image Image() const { return image; }
        vk::ImageView ImageView() const { return *imageView; }

    private:
        SwapchainImage(u32 index, vk::Image image,
                       vk::raii::ImageView imageView,
                       vk::raii::Semaphore imageAvailableSemaphore)
            : index(index), image(std::move(image)),
              imageView(std::move(imageView)),
              renderFinished(std::move(imageAvailableSemaphore))
        {
        }

        u32 index;
        vk::Image image;
        vk::raii::ImageView imageView;
        /** Indicates that the renderer has finished rendering and the image is
         * ready to be presented */
        vk::raii::Semaphore renderFinished;

        friend class Swapchain;
    };

    class Swapchain
    {
    public:
        Swapchain(Device &device, vk::SurfaceKHR surface,
                  vk::SwapchainKHR oldSwapChain = nullptr);

        SwapchainImage &AcquireNextImage(const vk::Semaphore semaphore);

        vk::Format ImageFormat() const { return vkImageFormat; }
        vk::Extent2D Extent() const { return extent; }
        vk::SwapchainKHR VkSwapChain() { return *vkSwapChain; }
        vk::SwapchainKHR operator*() const { return *vkSwapChain; }
        void Present(SwapchainImage &image);

    private:
        Device &device;
        vk::Queue presentQueue;
        vk::SurfaceKHR surface;
        vk::Format vkImageFormat;
        vk::Extent2D extent;
        vk::raii::SwapchainKHR vkSwapChain = nullptr;
        std::vector<SwapchainImage> swapchainImages;

        void CreateImages(std::vector<vk::Image> images);

        /** Create or recreate the swapchain. */
        void Recreate();
    };
} // namespace gd
