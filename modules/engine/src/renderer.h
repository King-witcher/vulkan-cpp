#pragma once

#include <vector>

#include "rhi.h"

#include "mesh.h"

namespace gd
{
    // Desenha a cena através da RHI. Toda a sincronização/apresentação vive
    // dentro do driver; aqui só há criação de pipeline e gravação de draws.
    class Renderer
    {
    public:
        explicit Renderer(rhi::Driver &driver);
        ~Renderer();

        void DrawScene(std::vector<Mesh> &scene);

    private:
        rhi::Driver &driver;
        rhi::PipelineHandle pipeline;
    };
} // namespace gd
