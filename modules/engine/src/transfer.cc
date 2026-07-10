#include "panic.h"

#include "transfer.h"
#include "unwrap.h"

namespace gd
{
    TransferContext::TransferContext(gd::Device &device, gd::Allocator &allocator)
        : device(&device), allocator(&allocator), queue(device.VkDevice().getQueue(device.GraphicsIndex(), 0)),
          pool(MakeCommandPool(device)),
          // Unsignaled: the first WaitForFence needs to actually wait for the GPU. If
          // it started signaled, the wait would return before the copy finished.
          fence(device.CreateFence(false))
    {
    }

    vk::raii::CommandPool TransferContext::MakeCommandPool(gd::Device &device)
    {
        vk::CommandPoolCreateInfo info;
        // eTransient: command buffers are short-lived (one upload and they're freed).
        info.setFlags(vk::CommandPoolCreateFlagBits::eTransient);
        info.setQueueFamilyIndex(device.GraphicsIndex());

        return Unwrap(device.VkDevice().createCommandPool(info), "Failed to create transfer command pool");
    }

    void TransferContext::ImmediateSubmit(std::function<void(vk::CommandBuffer)> record)
    {
        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.setCommandPool(pool);
        allocInfo.setCommandBufferCount(1);
        allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);

        auto buffers =
            Unwrap(device->VkDevice().allocateCommandBuffers(allocInfo), "Failed to allocate transfer command buffer");
        auto cmd = std::move(buffers[0]);

        cmd.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        record(*cmd);
        cmd.end();

        vk::CommandBufferSubmitInfo cmdInfo;
        cmdInfo.setCommandBuffer(*cmd);

        vk::SubmitInfo2 submitInfo;
        submitInfo.setCommandBufferInfos(cmdInfo);

        auto submitResult = queue.submit2(submitInfo, fence);
        if (submitResult != vk::Result::eSuccess)
            Panic("Failed to submit transfer command buffer");

        // Blocks until the GPU finishes and leaves the fence ready for the next use.
        if (device->WaitAndReset(fence) != vk::Result::eSuccess)
            Panic("Failed to wait on transfer fence");

        // `cmd` (RAII) returns to the pool when it goes out of scope.
    }

    gd::Buffer TransferContext::UploadBuffer(std::span<const u8> data, vk::BufferUsageFlags usage)
    {
        // 1. Host-visible staging: the CPU writes the data here.
        vk::BufferCreateInfo stagingInfo;
        stagingInfo.setSize(data.size());
        stagingInfo.setUsage(vk::BufferUsageFlagBits::eTransferSrc);
        stagingInfo.setSharingMode(vk::SharingMode::eExclusive);

        auto staging = allocator->Allocate(stagingInfo, AllocMode::HostVisible);
        if (staging.result != vk::Result::eSuccess)
            Panic("Failed to allocate staging buffer");
        staging.value.MapCopy(data);

        // 2. Device-local destination (VRAM), also a transfer target.
        vk::BufferCreateInfo gpuInfo;
        gpuInfo.setSize(data.size());
        gpuInfo.setUsage(usage | vk::BufferUsageFlagBits::eTransferDst);
        gpuInfo.setSharingMode(vk::SharingMode::eExclusive);

        auto gpu = allocator->Allocate(gpuInfo, AllocMode::DeviceLocal);
        if (gpu.result != vk::Result::eSuccess)
            Panic("Failed to allocate device-local buffer");

        // 3. Copies staging -> device-local on the GPU and waits for the copy to finish.
        ImmediateSubmit(
            [&](vk::CommandBuffer cmd)
            {
                vk::BufferCopy region;
                region.setSize(data.size());
                cmd.copyBuffer(staging.value.VkBuffer(), gpu.value.VkBuffer(), region);
            });

        // `staging` is destroyed here — safe, since ImmediateSubmit already waited
        // on the fence, guaranteeing the copy has finished.
        return std::move(gpu.value);
    }
} // namespace gd
