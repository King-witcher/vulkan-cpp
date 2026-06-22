#pragma once
#include "RustTypes.h"

#include <vector>

namespace vkwiz
{
  class Model
  {
  public:
    struct Vertex
    {
      f32 position[3];
      f32 color[3];
    };

    Model(const std::vector<Vertex>);
    std::vector<Vertex> vertices;
  };
}
