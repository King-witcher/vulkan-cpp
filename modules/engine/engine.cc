#include <iostream>

#include "engine.h"
#include "rust_types.h"
#include "mesh.h"

using namespace gd;

void gd::Engine::Run()
{
    std::vector<gd::Mesh::Vertex> vertices = {
        {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    };
    auto dataSize = vertices.size() * sizeof(gd::Mesh::Vertex);
    auto [buffer, mem] = device.Alloc(dataSize);
    auto ptr = *mem.mapMemory(0, dataSize);
    memcpy(ptr, vertices.data(), dataSize);
    mem.unmapMemory();
    for (;;)
    {
        Draw(buffer, vertices.size());
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

void gd::Engine::Draw(vk::raii::Buffer &vertexBuffer, u32 vertices)
{
    auto renderFrame = renderer.BeginFrame();
    renderFrame.Draw(vertexBuffer, vertices, pipeline);
    renderer.EndFrame(renderFrame);
}

gd::Engine::Engine()
{
    window.SetPosition(-1400, 200);
}
