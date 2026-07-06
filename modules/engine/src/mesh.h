#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "rust_types.h"

#include "allocator.h"
#include "vertex.h"

namespace gd
{
    class Mesh
    {
        friend class Renderer;

    public:
        Mesh(gd::Allocator &, const std::vector<Vertex> &);

        u32 vertexCount;

    private:
        static gd::Buffer MakeVertexBuffer(gd::Allocator &,
                                           const std::vector<Vertex> &);

        gd::Buffer buffer;
        // TODO: Review it. There is an allocation limit.
        vk::raii::DeviceMemory deviceMemory = nullptr;
    };
} // namespace gd
