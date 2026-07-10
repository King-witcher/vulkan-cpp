#include <vector>

#include "mesh.h"
#include "transfer.h"
#include "vertex.h"

namespace gd
{
    Mesh::Mesh(gd::TransferContext &transfer, const std::vector<gd::Vertex> &vertices)
        : vertexCount(vertices.size()), buffer(MakeVertexBuffer(transfer, vertices))
    {
    }

    gd::Buffer Mesh::MakeVertexBuffer(gd::TransferContext &transfer, const std::vector<gd::Vertex> &vertices)
    {
        // The vertex buffer lives in VRAM; TransferContext handles the staging.
        return transfer.UploadBuffer(vertices, vk::BufferUsageFlagBits::eVertexBuffer);
    }
} // namespace gd
