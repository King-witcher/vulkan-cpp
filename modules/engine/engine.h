#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "input.h"
#include "renderer.h"
#include "window.h"
#include "device.h"
#include "swapchain.h"
#include "pipeline.h"
#include "frame_in_flight.h"

namespace gd
{
    class Engine
    {
    public:
        Engine();

        void Run();

    private:
        gd::Window window{"Giuseppe"};
        vk::raii::Context vkContext;
        vk::raii::Instance vkInstance = CreateInstance();
        vk::raii::SurfaceKHR vkSurface = window.VulkanSurface(vkInstance);
        gd::Device device{vkInstance, vkSurface};
        gd::Swapchain swapChain{device, vkSurface};
        gd::Pipeline pipeline = {device, swapChain, "shaders/shader.spv"};
        gd::Input input{};

        gd::Renderer renderer{device, swapChain};

        vk::raii::Instance CreateInstance() const;
        void Draw(vk::raii::Buffer &vertexBuffer, u32 vertices);
    };
} // namespace gd
