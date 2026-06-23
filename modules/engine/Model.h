#pragma once
#include "RustTypes.h"
#include "glm/glm.hpp"
#include "Vulkan.h"

#include <vector>

namespace vkwiz
{
  class Model
  {
  public:
    struct Vertex
    {
      glm::vec3 position[3];
      glm::vec3 color[3];

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
        position.setOffset(offsetof(Vertex, position));

        return {position, color};
      }
    };

    Model(const std::vector<Vertex> vertices)
    {
      this->vertices = vertices;
    }

    std::vector<Vertex> vertices;
  };
}
