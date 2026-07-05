#include "frame_in_flight.h"

gd::FrameInFlight::FrameInFlight(gd::Device &device)
{
    auto commandBuffers = device.AllocateCommandBuffers(1);
    commandBuffer = std::move(commandBuffers[0]);
    imageAvailable = device.CreateSemaphore();
    fence = device.CreateFence(true);
}
