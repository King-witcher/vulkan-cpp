#pragma once

#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>

#include "pipeline.h"

namespace gd
{
  class Mesh
  {
  public:
    struct Vertex
    {
      glm::vec3 position;
      glm::vec3 color;

      static vk::VertexInputBindingDescription BindingDescription()
      {
        vk::VertexInputBindingDescription description;
        description.setBinding(0);
        description.setStride(sizeof(Vertex));
        description.setInputRate(vk::VertexInputRate::eVertex);

        return description;
      }

      static std::array<vk::VertexInputAttributeDescription, 2> AttributeDescriptions()
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

    Mesh(const std::vector<Vertex> vertices, gd::Pipeline &pipeline) : vertices(std::move(vertices)), pipeline(pipeline) {}

    std::vector<Vertex> vertices;
    gd::Pipeline &pipeline;

  private:
  };
}
