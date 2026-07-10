#pragma once

#include <functional>
#include <span>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "rust_types.h"

#include "allocator.h"
#include "device.h"

namespace gd
{
    class TransferContext
    {
    public:
        TransferContext(gd::Device &, gd::Allocator &);
        TransferContext(TransferContext &) = delete;
        TransferContext &operator=(TransferContext &) = delete;

        // Records a lambda into a disposable command buffer, submits it, and BLOCKS
        // until the GPU finishes. Used for uploads, layout transitions, etc.
        void ImmediateSubmit(std::function<void(vk::CommandBuffer)> record);

        // Allocates a device-local buffer (VRAM) and uploads `data` to it via staging.
        // The host-visible staging buffer is created and freed internally.
        gd::Buffer UploadBuffer(std::span<const u8> data, vk::BufferUsageFlags usage);

        template <typename T> inline gd::Buffer UploadBuffer(const std::vector<T> &vector, vk::BufferUsageFlags usage)
        {
            std::span<const u8> span{reinterpret_cast<const u8 *>(vector.data()), vector.size() * sizeof(T)};
            return UploadBuffer(span, usage);
        }

    private:
        gd::Device *device;
        gd::Allocator *allocator;
        vk::Queue queue;
        vk::raii::CommandPool pool;
        vk::raii::Fence fence;

        static vk::raii::CommandPool MakeCommandPool(gd::Device &);
    };
} // namespace gd
