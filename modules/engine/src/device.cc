#include "device.h"
#include "rust_types.h"
#include "panic.h"
#include "vulkan/vulkan.hpp"

#include <vector>
#include <array>

using namespace gd;

std::array REQUIRED_EXTENSIONS = {
    vk::KHRShaderDrawParametersExtensionName,
    vk::KHRCreateRenderpass2ExtensionName,
    vk::KHRSynchronization2ExtensionName,
    vk::KHRSwapchainExtensionName,
    vk::KHRSpirv14ExtensionName,
};

void InspectDevice(vk::raii::PhysicalDevice &device)
{
    using namespace std;

    auto properties = device.getProperties2().properties;
    auto memProperties = device.getMemoryProperties2().memoryProperties;

    cout << "Found device: " << properties.deviceName << " - "
         << properties.deviceID << endl;
    cout << "Max memory allocation count: "
         << properties.limits.maxMemoryAllocationCount << endl;
    cout << "Memory heaps: " << memProperties.memoryHeapCount << endl;
    cout << "Memory types: " << memProperties.memoryTypeCount << endl;

    for (u32 heapIndex = 0; heapIndex < memProperties.memoryHeapCount;
         heapIndex++)
    {
        auto &heap = memProperties.memoryHeaps[heapIndex];
        cout << "  Heap " << heapIndex << ": "
             << (heap.size / (1024.0 * 1024.0 * 1024.0))
             << " GB, flags: " << vk::to_string(heap.flags) << endl;

        for (u32 typeIndex = 0; typeIndex < memProperties.memoryTypeCount;
             typeIndex++)
        {
            auto &type = memProperties.memoryTypes[typeIndex];
            if (type.heapIndex != heapIndex)
                continue;

            cout << "    Memory type " << typeIndex << ": "
                 << vk::to_string(type.propertyFlags) << endl;
        }
    }
}

bool IsDeviceSuitable(vk::raii::PhysicalDevice device)
{
    auto properties = device.getProperties();
    auto features = device.getFeatures();

    if (properties.apiVersion < VK_API_VERSION_1_4)
        return false;

    if (properties.limits.maxPushConstantsSize < 128)
        return false;

    if (properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu)
        return false;

    if (!features.geometryShader)
        return false;

    return true;
}

vk::raii::PhysicalDevice PickPhysicalDevice(vk::raii::Instance &instance)
{
    auto [result, devices] = instance.enumeratePhysicalDevices();
    if (result != vk::Result::eSuccess)
        Panic("failed to enumerate physical devices");
    if (devices.size() == 0)
        Panic("no vulkan compatible GPU found");

    // TODO: Pick the most suitable device
    for (const auto &device : devices)
    {
        if (IsDeviceSuitable(device))
            return device;
    }
    Panic("failed to find a suitable GPU!");
}

/** Gets the index of the first queue family that supports graphics in a
 * specific device. */
u32 FindGraphicsQueueFamily(vk::raii::PhysicalDevice device)
{
    auto familyProperties = device.getQueueFamilyProperties();

    for (u32 i = 0; i < familyProperties.size(); i++)
    {
        auto familyProperty = familyProperties[i];
        if (familyProperty.queueFlags & vk::QueueFlagBits::eGraphics)
            return i;
    }
    Panic("unreachable: Vulkan requires implementations to expose at least one "
          "graphics queue family");
}

vk::raii::Device CreateLogicalDevice(vk::raii::PhysicalDevice physicalDevice,
                                     u32 graphicsIndex)
{
    using namespace vk;

    std::array queuePriorities = {0.5f};
    DeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.setQueuePriorities(queuePriorities);
    std::array queueCreateInfos = {queueCreateInfo};

    StructureChain<PhysicalDeviceFeatures2, PhysicalDeviceVulkan13Features,
                   PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        featureChain = {
            {}, // vk::PhysicalDeviceFeatures2 (empty for now)
            {
                .synchronization2 = true,
                .dynamicRendering = true,
            }, // Enable dynamic rendering from Vulkan 1.3
            {.extendedDynamicState =
                 true} // Enable extended dynamic state from the extension
        };

    DeviceCreateInfo deviceInfo{};
    deviceInfo.setPNext(&featureChain.get());
    deviceInfo.setQueueCreateInfos(queueCreateInfos);
    deviceInfo.setPEnabledExtensionNames(REQUIRED_EXTENSIONS);

    auto createResult = physicalDevice.createDevice(deviceInfo);
    if (!createResult.has_value())
    {
        Panic("failed to create device");
    }

    return std::move(*createResult);
}

vk::raii::CommandPool CreateCommandPool(vk::raii::Device &vkDevice,
                                        u32 graphicsIndex)
{
    using namespace vk;
    CommandPoolCreateInfo createInfo;
    createInfo.setFlags(CommandPoolCreateFlagBits::eResetCommandBuffer);
    createInfo.setQueueFamilyIndex(graphicsIndex);

    auto createResult = vkDevice.createCommandPool(createInfo);
    if (createResult.has_value())
        return std::move(*createResult);
    Panic("failed to create command pool");
}

gd::Device::Device(vk::raii::Instance &instance, vk::raii::SurfaceKHR &surface)
{
    vkPhysicalDevice = PickPhysicalDevice(instance);
#if _DEBUG
    InspectDevice(vkPhysicalDevice);
#endif
    auto graphicsIndex = FindGraphicsQueueFamily(vkPhysicalDevice);

    // TODO: Consider different queue families for presentation
    auto [surfaceSupportResult, surfaceSupport] =
        vkPhysicalDevice.getSurfaceSupportKHR(graphicsIndex, surface);
    if (surfaceSupportResult != vk::Result::eSuccess)
        Panic("Failed to get surface support for physical device.");
    if (surfaceSupport == vk::False)
        Panic("Selected physical device does not support presentation to the "
              "given surface.");

    vkDevice = CreateLogicalDevice(vkPhysicalDevice, graphicsIndex);
    vkCommandPool = CreateCommandPool(vkDevice, graphicsIndex);
    vkGraphicsQueue = vkDevice.getQueue(graphicsIndex, 0);
    vkPresentQueue = vkGraphicsQueue;
}

/** Gets information about the surface support for the physical device */
SurfaceSupport gd::Device::QuerySurfaceSupport(vk::SurfaceKHR surface)
{
    auto capabilities = vkPhysicalDevice.getSurfaceCapabilitiesKHR(surface);
    auto formats = vkPhysicalDevice.getSurfaceFormatsKHR(surface);
    auto presentModes = vkPhysicalDevice.getSurfacePresentModesKHR(surface);

    if (capabilities.result != vk::Result::eSuccess)
        Panic("Failed to get surface capabilities for physical device.");
    if (formats.result != vk::Result::eSuccess)
        Panic("Failed to get surface formats for physical device.");
    if (presentModes.result != vk::Result::eSuccess)
        Panic("Failed to get surface present modes for physical device.");

    return SurfaceSupport{
        .capabilities = *capabilities,
        .formats = *formats,
        .presentModes = *presentModes,
    };
}

void gd::Device::ResetFence(vk::raii::Fence &fence)
{
    vkDevice.resetFences(*fence);
}

vk::Result gd::Device::WaitForFence(vk::raii::Fence &fence)
{
    return vkDevice.waitForFences(*fence, vk::True, UINT64_MAX);
}

void gd::Device::SubmitGraphics(vk::SubmitInfo submitInfo, vk::Fence fence)
{
    vkGraphicsQueue.submit(submitInfo, fence);
}

void gd::Device::SubmitGraphics2(vk::SubmitInfo2 submitInfo, vk::Fence fence)
{
    vkGraphicsQueue.submit2(submitInfo, fence);
}

bool gd::Device::Present(vk::PresentInfoKHR &presentInfo)
{
    auto result = vkPresentQueue.presentKHR(presentInfo);
    switch (result)
    {
    case vk::Result::eSuccess:
        return true;
    case vk::Result::eErrorOutOfDateKHR:
    case vk::Result::eSuboptimalKHR:
        return false;
    default:
        Panic("failed to present swapchain image");
    }
}

std::vector<vk::raii::CommandBuffer>
gd::Device::AllocateCommandBuffers(u32 count) const
{
    vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = vkCommandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = count,
    };
    return std::move(*vkDevice.allocateCommandBuffers(allocateInfo));
}

vk::raii::Semaphore gd::Device::CreateSemaphore() const
{
    return std::move(*vkDevice.createSemaphore({}));
}

vk::raii::Fence gd::Device::CreateFence(bool signaled) const
{
    return std::move(*vkDevice.createFence({
        .flags = signaled ? vk::FenceCreateFlagBits::eSignaled
                          : vk::FenceCreateFlags{},
    }));
}

vk::raii::ShaderModule
gd::Device::CreateShaderModule(const std::vector<u8> code) const
{
    vk::ShaderModuleCreateInfo createInfo{
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const u32 *>(code.data()),
    };
    return std::move(*vkDevice.createShaderModule(createInfo));
}

std::tuple<vk::raii::Buffer, vk::raii::DeviceMemory>
gd::Device::Allocate(usize size)
{
    // Create buffer
    vk::BufferCreateInfo bufferInfo;
    bufferInfo.setSize(size);
    bufferInfo.setUsage(vk::BufferUsageFlagBits::eVertexBuffer);
    bufferInfo.setSharingMode(vk::SharingMode::eExclusive);
    auto bufferResult = vkDevice.createBuffer(bufferInfo);
    if (!bufferResult.has_value())
        Panic("failed to create buffer");
    auto buffer = std::move(*bufferResult);

    // Allocate memory
    auto requirements = buffer.getMemoryRequirements();
    // Estamos usando HostCoherent para não precisar dar
    // vkDevice.mapFlushedMemoryRanges() depois de escrever na memória mapeada e
    // vkDevice.invalidateMappedMemoryRanges antes de ler da memória mapeada.
    // Mas isso tem desempenho pior e pode ser mudado depois.
    auto memType =
        FindMemoryType(requirements.memoryTypeBits,
                       vk::MemoryPropertyFlagBits::eHostVisible |
                           vk::MemoryPropertyFlagBits::eHostCoherent);
    vk::MemoryAllocateInfo memInfo;
    memInfo.setAllocationSize(requirements.size);
    memInfo.setMemoryTypeIndex(memType);
    auto memResult = vkDevice.allocateMemory(memInfo);
    if (!memResult.has_value())
        Panic("failed to allocate memory for buffer");
    auto memory = std::move(*memResult);

    // Vincula a memória alocada ao buffer. Sem isso, o buffer não tem
    // armazenamento e qualquer uso dele dispara VUID-...-pBuffers-00628.
    buffer.bindMemory(*memory, 0);

    return std::make_tuple(std::move(buffer), std::move(memory));
}

// Existem heaps diferentes como VRAM e espaço de swap na RAM pra quando a VRAM
// acaba. São heaps diferentes. Dentro de cada heap, existem tipos diferentes de
// memória.
u32 gd::Device::FindMemoryType(u32 supportedTypes,
                               vk::MemoryPropertyFlags properties)
{
    auto memProps = vkPhysicalDevice.getMemoryProperties2();
    for (u32 i = 0; i < memProps.memoryProperties.memoryTypeCount; i++)
    {
        if ((supportedTypes & (1 << i)) && // Buffer suporta tipo i?
            (memProps.memoryProperties.memoryTypes[i].propertyFlags &
             properties) ==
                properties) // Tipo i tem todas as flags que eu pedi?
        {
            return i;
        }
    }

    Panic("failed to find suitable memory type");
}
