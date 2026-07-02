#include "Frame.h"

gd::Frame::Frame(gd::Device &device)
{
    auto commandBuffers = device.allocateCommandBuffers(1);
    commandBuffer = std::move(commandBuffers[0]);
    presentReady = device.createSemaphore();
    inFlightFence = device.createFence(true);
}
