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
        Mesh(gd::TransferContext &, const std::vector<Vertex> &);

        gd::Buffer &VertexBuffer() { return buffer; }

        u32 vertexCount;

    private:
        static gd::Buffer MakeVertexBuffer(gd::TransferContext &, const std::vector<Vertex> &);

        gd::Buffer buffer;
    };
} // namespace gd
