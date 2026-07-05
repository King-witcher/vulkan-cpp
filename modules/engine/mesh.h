#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "device.h"
#include "vertex.h"

namespace gd
{
    class Mesh
    {
        friend class Renderer;

    public:
        Mesh(gd::Device &device, const std::vector<Vertex> vertices);

        std::vector<Vertex> vertices;

    private:
        vk::raii::Buffer vertexBuffer = nullptr;
        // TODO: Review it. There is an allocation limit.
        vk::raii::DeviceMemory deviceMemory = nullptr;
    };
} // namespace gd
