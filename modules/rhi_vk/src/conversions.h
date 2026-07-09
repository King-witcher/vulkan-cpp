#pragma once

// Translates RHI agnostic enums into Vulkan specific values

#include <vulkan/vulkan.hpp>

#include "panic.h"
#include "rhi.h"

namespace gd::rhi::vulkan
{
    inline vk::Format ToVk(VertexFormat format)
    {
        switch (format)
        {
        case VertexFormat::Float32x2:
            return vk::Format::eR32G32Sfloat;
        case VertexFormat::Float32x3:
            return vk::Format::eR32G32B32Sfloat;
        case VertexFormat::Float32x4:
            return vk::Format::eR32G32B32A32Sfloat;
        }
        Panic("unknown vertex format");
    }

    inline vk::PrimitiveTopology ToVk(PrimitiveTopology topology)
    {
        switch (topology)
        {
        case PrimitiveTopology::TriangleList:
            return vk::PrimitiveTopology::eTriangleList;
        }
        Panic("unknown primitive topology");
    }

    inline vk::CullModeFlags ToVk(CullMode cull)
    {
        switch (cull)
        {
        case CullMode::None:
            return vk::CullModeFlagBits::eNone;
        case CullMode::Back:
            return vk::CullModeFlagBits::eBack;
        case CullMode::Front:
            return vk::CullModeFlagBits::eFront;
        }
        Panic("unknown cull mode");
    }

    inline vk::BufferUsageFlags ToVk(BufferUsage usage)
    {
        switch (usage)
        {
        case BufferUsage::Vertex:
            return vk::BufferUsageFlagBits::eVertexBuffer;
        case BufferUsage::Index:
            return vk::BufferUsageFlagBits::eIndexBuffer;
        }
        Panic("unknown buffer usage");
    }
} // namespace gd::rhi::vulkan
