#include <iostream>

#include "Engine.h"
#include "RustTypes.h"
#include "Input.h"
#include "Mesh.h"

using namespace gd;

void gd::Engine::run()
{
    std::vector<gd::Mesh::Vertex> vertices = {
        {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    };
    auto dataSize = vertices.size() * sizeof(gd::Mesh::Vertex);
    auto [buffer, mem] = device_.alloc(dataSize);
    auto ptr = *mem.mapMemory(0, dataSize);
    memcpy(ptr, vertices.data(), dataSize);
    mem.unmapMemory();
    for (;;)
    {
        draw(buffer, vertices.size());
        input::update();
        if (input::shouldQuit())
            break;
    }
    device_.vkDevice().waitIdle();
    std::cout << "Exiting engine loop." << std::endl;
}

vk::raii::Instance gd::Engine::createInstance() const
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

    auto requiredExtensions = window_.getRequiredVulkanExtensions();
    // TODO: check for supported extensions
    vk::InstanceCreateInfo createInfo{};
    createInfo.setPApplicationInfo(&appInfo);
    createInfo.setPEnabledLayerNames(layers);
    createInfo.setPEnabledExtensionNames(requiredExtensions);

    auto instance = std::move(*vkContext_.createInstance(createInfo));
    std::cout << "Vulkan instance created." << std::endl;
    return instance;
}

void gd::Engine::draw(vk::raii::Buffer &vertexBuffer, u32 vertices)
{
    auto renderFrame = renderer.beginFrame();
    renderFrame.draw(vertexBuffer, vertices, pipeline_);
    renderer.endFrame(renderFrame);
}

gd::Engine::Engine()
{
    window_.setPosition(-1400, 200);
}
