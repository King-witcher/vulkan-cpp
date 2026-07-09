#pragma once

#include <vector>

#include "rhi.h"
#include "rust_types.h"

#include "vertex.h"

namespace gd
{
    class Mesh
    {
        friend class Renderer;

    public:
        Mesh(rhi::Driver &driver, const std::vector<Vertex> &vertices);
        ~Mesh();

        Mesh(Mesh &&) noexcept;
        Mesh &operator=(Mesh &&) noexcept;
        Mesh(const Mesh &) = delete;
        Mesh &operator=(const Mesh &) = delete;

    private:
        rhi::Driver *driver;
        rhi::BufferHandle buffer;
        u32 vertexCount;
    };
} // namespace gd
