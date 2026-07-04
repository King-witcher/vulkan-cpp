#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "Frame.h"
#include "Device.h"
#include "SwapChain.h"
#include "Pipeline.h"
#include "Frame.h"

namespace gd
{
    class Renderer
    {

    public:
        Renderer(gd::Device &device);

    private:
        static const u32 MAX_FRAMES_IN_FLIGHT = 2;

        gd::Device &device;
        gd::Swapchain &swapchain;
        gd::Pipeline &pipeline;

        std::array<gd::Frame, MAX_FRAMES_IN_FLIGHT> frames = {
            gd::Frame(device),
            gd::Frame(device)};
    };
}
