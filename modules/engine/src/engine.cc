#include <iostream>
#include <memory>
#include <vector>

#include "rhi.h"
#include "rhi_vk.h"

#include "input.h"
#include "renderer.h"
#include "vertex.h"
#include "window.h"
#include "engine/engine.h"

namespace gd
{
    class Engine::Impl
    {
    public:
        Impl() { window.SetPosition(-1400, 200); }

        void Run()
        {
            std::vector<gd::Vertex> vertices1 = {
                {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            };

            std::vector<gd::Vertex> vertices2 = {
                {{0.2f, -0.4f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                {{0.2f, 0.6f, 0.1f}, {0.0f, 1.0f, 0.0f}},
                {{-0.6f, 0.6f, 0.1f}, {0.0f, 0.0f, 1.0f}},
            };

            std::vector<gd::Mesh> meshes;
            meshes.emplace_back(*driver, vertices1);
            meshes.emplace_back(*driver, vertices2);

            for (;;)
            {
                renderer.DrawScene(meshes);
                input.Update();
                if (input.ShouldQuit())
                    break;
            }
            driver->WaitIdle();
            std::cout << "Exiting engine loop." << std::endl;
        }

    private:
        Window window{"Giuseppe"};
        Input input{};
        // Escolha do backend acontece aqui — trocar por outra fábrica (ex.:
        // sdlgpu::CreateDriver) migraria a engine inteira de API gráfica.
        std::unique_ptr<rhi::Driver> driver =
            rhi::vulkan::CreateDriver(window.SdlHandle());
        Renderer renderer{*driver};
    };

    Engine::Engine() : impl(new Impl) {}

    void Engine::Run()
    {
        impl->Run();
    }

    Engine::~Engine() = default;
} // namespace gd
