#pragma once

#include "Vulkan.h"
#include <glm/glm.hpp>

namespace vkwiz
{
  using namespace glm;
  class Model
  {
  public:
    struct Vertex
    {
      vec2 pos;
      vec3 color;

      static vk::VertexInputBindingDescription bindingDescription()
      {
        auto result = vk::VertexInputBindingDescription{};
        result.setBinding(0);
        result.setStride(sizeof(Vertex));
        result.setInputRate(vk::VertexInputRate::eVertex);
        return result;
      }

      static std::array<vk::VertexInputAttributeDescription, 2> attributeDescription()
      {
        std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].setBinding(0);
        attributeDescriptions[0].setLocation(0);
        attributeDescriptions[0].setFormat(vk::Format::eR32G32Sfloat);
        attributeDescriptions[0].setOffset(offsetof(Vertex, pos));

        attributeDescriptions[1].setBinding(0);
        attributeDescriptions[1].setLocation(1);
        attributeDescriptions[1].setFormat(vk::Format::eR32G32B32Sfloat);
        attributeDescriptions[1].setOffset(offsetof(Vertex, color));

        return attributeDescriptions;
      }
    };

    Model();

  private:
  };
}
