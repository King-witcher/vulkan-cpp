#include <iostream>

#include "allocator.h"
#include "device.h"
#include "engine/engine.h"
#include "input.h"
#include "renderer.h"
#include "swapchain.h"
#include "unwrap.h"
#include "window.h"

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
            meshes.emplace_back(allocator, vertices1);
            meshes.emplace_back(allocator, vertices2);

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
        vk::raii::SurfaceKHR vkSurface{vkInstance, window.VulkanSurface(*vkInstance)};

        Device device{vkInstance, vkSurface};
        Allocator allocator{vkInstance, device.PhysicalDevice(), device.VkDevice()};
        Swapchain swapchain{device, vkSurface};
        Input input{};
        Renderer renderer{device, swapchain};

        vk::raii::Instance CreateInstance() const
        {
            vk::ApplicationInfo appInfo;
            appInfo.setPApplicationName("GLEED Test");
            appInfo.setApplicationVersion(vk::makeVersion(1, 0, 0));
            appInfo.setPEngineName("GLEED 1");
            appInfo.setEngineVersion(vk::makeVersion(1, 0, 0));
            appInfo.setApiVersion(vk::ApiVersion14);

#ifdef _DEBUG
            auto layers = std::vector<const char *>{"VK_LAYER_KHRONOS_validation"};
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

            auto instance = Unwrap(vkContext.createInstance(createInfo), "failed to create Vulkan instance");
            std::cout << "Vulkan instance created." << std::endl;
            return instance;
        };

        void Draw(std::vector<gd::Mesh> &meshes)
        {
            auto renderpass = renderer.BeginRenderPass();
            renderer.DrawScene(renderpass, meshes);
            renderer.SubmitFrame(std::move(renderpass));
        };
    };

    Engine::Engine() : impl(new Impl) {}

    void Engine::Run()
    {
        impl->Run();
    }

    Engine::~Engine() = default;
} // namespace gd
