#include <iostream>

#include "device.h"
#include "input.h"
#include "renderer.h"
#include "swapchain.h"
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
            meshes.emplace_back(device, vertices1);
            meshes.emplace_back(device, vertices2);

            for (;;)
            {
                Draw(meshes);
                input.Update();
                if (input.ShouldQuit())
                    break;
            }
            device.VkDevice().waitIdle();
            std::cout << "Exiting engine loop." << std::endl;
        }

    private:
        Window window{"Giuseppe"};

        vk::raii::Context vkContext;
        vk::raii::Instance vkInstance = CreateInstance();
        vk::raii::SurfaceKHR vkSurface = window.VulkanSurface(vkInstance);

        Device device{vkInstance, vkSurface};
        Swapchain swapchain{device, vkSurface};
        Input input{};
        Renderer renderer{device, swapchain};

        vk::raii::Instance CreateInstance() const
        {
            vk::ApplicationInfo appInfo;
            appInfo.setPApplicationName("VkWizard");
            appInfo.setApplicationVersion(vk::makeVersion(1, 0, 0));
            appInfo.setPEngineName("No Engine");
            appInfo.setEngineVersion(vk::makeVersion(1, 0, 0));
            appInfo.setApiVersion(vk::ApiVersion14);

#ifdef _DEBUG
            auto layers =
                std::vector<const char *>{"VK_LAYER_KHRONOS_validation"};
            std::cout << "Enabling validation layers..." << std::endl;
#else
            auto layers = std::vector<const char *>{};
#endif

            auto requiredExtensions = window.RequiredVulkanExtensions();
            // TODO: check for supported extensions
            vk::InstanceCreateInfo createInfo{};
            createInfo.setPApplicationInfo(&appInfo);
            createInfo.setPEnabledLayerNames(layers);
            createInfo.setPEnabledExtensionNames(requiredExtensions);

            auto instance = std::move(*vkContext.createInstance(createInfo));
            std::cout << "Vulkan instance created." << std::endl;
            return instance;
        };

        void Draw(std::vector<gd::Mesh> &meshes)
        {
            auto frame = renderer.BeginFrame();
            renderer.DrawScene(frame, meshes);
            renderer.EndFrame(frame);
        };
    };

    Engine::Engine() : impl(new Impl) {}

    void Engine::Run()
    {
        impl->Run();
    }

    Engine::~Engine() = default;
} // namespace gd
