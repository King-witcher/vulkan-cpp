#include <iostream>
#include <vector>

#include "engine.h"
#include "mesh.h"

using namespace gd;

void gd::Engine::Run()
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

vk::raii::Instance gd::Engine::CreateInstance() const
{
    vk::ApplicationInfo appInfo;
    appInfo.setPApplicationName("VkWizard");
    appInfo.setApplicationVersion(vk::makeVersion(1, 0, 0));
    appInfo.setPEngineName("No Engine");
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

    auto instance = std::move(*vkContext.createInstance(createInfo));
    std::cout << "Vulkan instance created." << std::endl;
    return instance;
}

void gd::Engine::Draw(std::vector<gd::Mesh> &meshes)
{
    auto frame = renderer.BeginFrame();
    renderer.DrawScene(frame, meshes);
    renderer.EndFrame(frame);
}

gd::Engine::Engine()
{
    window.SetPosition(-1400, 200);
}
