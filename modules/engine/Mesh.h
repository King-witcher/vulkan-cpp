#pragma once
#include "RustTypes.h"
#include "glm/glm.hpp"
#include <vulkan/vulkan_raii.hpp>

#include <vector>

namespace gd
{
  class Mesh
  {
  public:
    struct Vertex
    {
      glm::vec3 position;
      glm::vec3 color;

      static vk::VertexInputBindingDescription getBindingDescription()
      {
        vk::VertexInputBindingDescription description;
        description.setBinding(0);
        description.setStride(sizeof(Vertex));
        description.setInputRate(vk::VertexInputRate::eVertex);

        return description;
      }

      static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions()
      {
        vk::VertexInputAttributeDescription position;
        position.setLocation(0);
        position.setBinding(0);
        position.setFormat(vk::Format::eR32G32B32Sfloat);
        position.setOffset(offsetof(Vertex, position));

        vk::VertexInputAttributeDescription color;
        color.setLocation(1);
        color.setBinding(0);
        color.setFormat(vk::Format::eR32G32B32Sfloat);
        color.setOffset(offsetof(Vertex, color));

        return {position, color};
      }
    };

    Mesh(const std::vector<Vertex> vertices) : vertices(std::move(vertices)) {}

    std::vector<Vertex> vertices;
  };
}
