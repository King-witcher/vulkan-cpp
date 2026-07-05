#include "mesh.h"
#include "device.h"
#include "panic.h"
#include "vertex.h"
#include "vulkan/vulkan.hpp"
#include <cstring>
#include <vector>

namespace gd
{
    Mesh::Mesh(gd::Device &device, std::vector<gd::Vertex> vertices)
        : vertices(vertices)
    {
        auto size = vertices.size() * sizeof(Vertex);
        auto [buffer, memory] = device.Alloc(size);

        auto [ptrResult, ptr] = memory.mapMemory(0, size);
        if (ptrResult != vk::Result::eSuccess)
            Panic("failed to map memory");
        memcpy(ptr, vertices.data(), size);
        memory.unmapMemory();

        vertexBuffer = std::move(buffer);
        deviceMemory = std::move(memory);
    }
} // namespace gd
