#include <vector>

#include "panic.h"

#include "allocator.h"
#include "mesh.h"
#include "vertex.h"

namespace gd
{
    Mesh::Mesh(gd::Allocator &allocator,
               const std::vector<gd::Vertex> &vertices)
        : vertexCount(vertices.size()),
          buffer(MakeVertexBuffer(allocator, vertices))
    {
    }

    gd::Buffer Mesh::MakeVertexBuffer(gd::Allocator &allocator,
                                      const std::vector<gd::Vertex> &vertices)
    {
        vk::BufferCreateInfo bufferInfo;
        bufferInfo.setSize(vertices.size() * sizeof(Vertex));
        bufferInfo.setUsage(vk::BufferUsageFlagBits::eVertexBuffer);
        bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

        auto allocated = allocator.Allocate(bufferInfo);
        if (allocated.result != vk::Result::eSuccess)
            Panic("Failed to allocate buffer");

        allocated.value.Write(vertices);
        return std::move(allocated.value);
    }
} // namespace gd
