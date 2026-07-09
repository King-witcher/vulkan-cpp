#pragma once

#include <array>

#include <glm/glm.hpp>

#include "rhi.h"

namespace gd
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;

        // O array precisa ter tempo de vida >= o span devolvido — por isso é
        // static/constexpr.
        static rhi::VertexLayout Layout()
        {
            static constexpr std::array attributes = {
                rhi::VertexAttribute{
                    .location = 0,
                    .format = rhi::VertexFormat::Float32x3,
                    .offset = offsetof(Vertex, position),
                },
                rhi::VertexAttribute{
                    .location = 1,
                    .format = rhi::VertexFormat::Float32x3,
                    .offset = offsetof(Vertex, color),
                },
            };
            return rhi::VertexLayout{
                .stride = sizeof(Vertex),
                .attributes = attributes,
            };
        }
    };
} // namespace gd
