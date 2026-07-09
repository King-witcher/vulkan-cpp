#include <cstring>
#include <utility>

#include "mesh.h"

namespace gd
{
    Mesh::Mesh(rhi::Driver &driver, const std::vector<Vertex> &vertices)
        : driver(&driver), vertexCount(static_cast<u32>(vertices.size()))
    {
        rhi::BufferDesc desc{
            .size = vertices.size() * sizeof(Vertex),
            .usage = rhi::BufferUsage::Vertex,
        };
        buffer = driver.CreateBuffer(desc);

        auto bytes = std::span<const u8>(
            reinterpret_cast<const u8 *>(vertices.data()), desc.size);
        driver.WriteBuffer(buffer, bytes);
    }

    Mesh::~Mesh()
    {
        if (driver && buffer)
            driver->DestroyBuffer(buffer);
    }

    Mesh::Mesh(Mesh &&other) noexcept
        : driver(other.driver), buffer(other.buffer),
          vertexCount(other.vertexCount)
    {
        other.buffer = {};
    }

    Mesh &Mesh::operator=(Mesh &&other) noexcept
    {
        if (this != &other)
        {
            if (driver && buffer)
                driver->DestroyBuffer(buffer);
            driver = other.driver;
            buffer = other.buffer;
            vertexCount = other.vertexCount;
            other.buffer = {};
        }
        return *this;
    }
} // namespace gd
