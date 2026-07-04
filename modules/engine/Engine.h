#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "Renderer.h"
#include "Window.h"
#include "Device.h"
#include "SwapChain.h"
#include "Pipeline.h"
#include "FrameInFlight.h"

namespace gd
{
    const u32 MAX_FRAMES_IN_FLIGHT = 2;

    class Engine
    {
    public:
        Engine();

        void run();

    private:
        gd::Window window_{"Giuseppe"};
        vk::raii::Context vkContext_;
        vk::raii::Instance vkInstance_ = createInstance();
        vk::raii::SurfaceKHR vkSurface_ = window_.getVulkanSurface(vkInstance_);
        gd::Device device_{vkInstance_, vkSurface_};
        gd::Swapchain swapChain_{device_, vkSurface_};
        gd::Pipeline pipeline_ = {device_, swapChain_, "shaders/shader.spv"};
        // u32 inFlightIndex = 0;

        // std::array<gd::FrameInFlight, MAX_FRAMES_IN_FLIGHT> frames = {
        // 	gd::FrameInFlight(device_),
        // 	gd::FrameInFlight(device_)};

        gd::Renderer renderer{device_, swapChain_, pipeline_};

        vk::raii::Instance createInstance() const;
        void recordCommandBuffer(
            vk::raii::CommandBuffer &commandBuffer,
            vk::Image image,
            vk::ImageView imageView,
            vk::raii::Buffer &vertexBuffer,
            u32 vertices);
        void draw(vk::raii::Buffer &vertexBuffer, u32 vertices);
    };
} // namespace gd
