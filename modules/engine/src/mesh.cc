#include <vector>

#include "mesh.h"
#include "transfer.h"
#include "vertex.h"

namespace gd
{
    Mesh::Mesh(gd::TransferContext &transfer, const std::vector<gd::Vertex> &vertices, const std::vector<u32> &indices)
        : vertexCount(vertices.size()), indexCount(indices.size()), vertexBuffer(MakeVertexBuffer(transfer, vertices)),
          indexBuffer(MakeIndexBuffer(transfer, indices))
    {
    }

    gd::Buffer Mesh::MakeVertexBuffer(gd::TransferContext &transfer, const std::vector<gd::Vertex> &vertices)
    {
        // The vertex buffer lives in VRAM; TransferContext handles the staging.
        return transfer.UploadBuffer(vertices, vk::BufferUsageFlagBits::eVertexBuffer);
    }

    gd::Buffer Mesh::MakeIndexBuffer(gd::TransferContext &transfer, const std::vector<u32> &indices)
    {
        // The index buffer lives in VRAM; TransferContext handles the staging.
        return transfer.UploadBuffer(indices, vk::BufferUsageFlagBits::eIndexBuffer);
    }
} // namespace gd
