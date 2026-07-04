#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "Device.h"
#include <vector>

namespace gd
{
    struct SwapchainImage
    {
    public:
        vk::Semaphore getRenderReadySemaphore() const { return *renderFinished; }
        vk::Image getImage() const { return image; }
        vk::ImageView getImageView() const { return *imageView; }

    private:
        SwapchainImage(u32 index, vk::Image image, vk::raii::ImageView imageView, vk::raii::Semaphore imageAvailableSemaphore)
            : index(index), image(std::move(image)), imageView(std::move(imageView)), renderFinished(std::move(imageAvailableSemaphore)) {}

        u32 index;
        vk::Image image;
        vk::raii::ImageView imageView;
        /** Indicates that the renderer has finished rendering and the image is ready to be presented */
        vk::raii::Semaphore renderFinished;

        friend class Swapchain;
    };

    class Swapchain
    {
    public:
        Swapchain(Device &device, vk::raii::SurfaceKHR &surface, vk::SwapchainKHR oldSwapChain = nullptr);

        SwapchainImage &acquireNextImage(const vk::Semaphore semaphore);

        vk::Format imageFormat() const { return vkImageFormat_; }
        vk::Extent2D extent() const { return extent_; }
        vk::raii::SwapchainKHR &vkSwapChain() { return vkSwapChain_; }
        vk::SwapchainKHR operator*() const { return *vkSwapChain_; }
        void present(gd::SwapchainImage &image);

    private:
        Device &device_;
        vk::SurfaceKHR surface_;
        vk::Format vkImageFormat_;
        vk::Extent2D extent_;
        vk::raii::SwapchainKHR vkSwapChain_ = nullptr;
        std::vector<SwapchainImage> swapchainImages;

        void createImages(std::vector<vk::Image> images);

        /** Create or recreate the swapchain. */
        void recreate();
    };
} // namespace gd
