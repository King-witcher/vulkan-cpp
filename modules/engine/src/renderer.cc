#include <fstream>
#include <string>
#include <vector>

#include "panic.h"

#include "renderer.h"
#include "vertex.h"

namespace gd
{
    static std::vector<u8> ReadFile(const std::string &filename)
    {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open())
            Panic("Failed to open file: " + filename);
        std::vector<u8> buffer(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        return buffer;
    }

    Renderer::Renderer(rhi::Driver &driver) : driver(driver)
    {
        // O SPIR-V do slang traz os dois estágios (vertMain/fragMain) no mesmo
        // módulo; passamos o mesmo blob para ambos com entry points distintos.
        auto spirv = ReadFile("shaders/shader.spv");

        rhi::PipelineDesc desc{
            .vertexShader = {.spirv = spirv, .entryPoint = "vertMain"},
            .fragmentShader = {.spirv = spirv, .entryPoint = "fragMain"},
            .vertexLayout = Vertex::Layout(),
            .topology = rhi::PrimitiveTopology::TriangleList,
            .cullMode = rhi::CullMode::Back,
        };
        pipeline = driver.CreatePipeline(desc);
    }

    Renderer::~Renderer()
    {
        driver.DestroyPipeline(pipeline);
    }

    void Renderer::DrawScene(std::vector<Mesh> &scene)
    {
        auto &pass = driver.BeginFrame({0.05f, 0.05f, 0.05f, 1.0f});

        pass.BindPipeline(pipeline);
        for (auto &mesh : scene)
        {
            pass.BindVertexBuffer(mesh.buffer);
            pass.Draw(mesh.vertexCount);
        }

        driver.EndFrame();
    }
} // namespace gd
