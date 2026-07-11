#pragma once

#include <glm/glm.hpp>

#include "rust_types.h"

#include "allocator.h"
#include "transfer.h"
#include "vertex.h"

namespace gd
{
    class Mesh
    {
    public:
        Mesh(gd::TransferContext &, const std::vector<Vertex> &, const std::vector<u32> &);

        gd::Buffer &VertexBuffer() { return vertexBuffer; }
        gd::Buffer &IndexBuffer() { return indexBuffer; }

        // Make private
        u32 vertexCount;
        u32 indexCount;

    private:
        static gd::Buffer MakeVertexBuffer(gd::TransferContext &, const std::vector<Vertex> &);
        static gd::Buffer MakeIndexBuffer(gd::TransferContext &, const std::vector<u32> &);

        gd::Buffer vertexBuffer;
        gd::Buffer indexBuffer;
    };
} // namespace gd
