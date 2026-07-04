#include "FrameInFlight.h"

gd::FrameInFlight::FrameInFlight(gd::Device &device)
{
    auto commandBuffers = device.allocateCommandBuffers(1);
    commandBuffer = std::move(commandBuffers[0]);
    imageAvailable = device.createSemaphore();
    fence = device.createFence(true);
}
