#include <array>
#include <iostream>
#include <vector>

#include "vulkan/vulkan.hpp"

#include "panic.h"
#include "rust_types.h"

#include "device.h"

namespace gd::rhi::vulkan
{
    static std::array REQUIRED_EXTENSIONS = {
        vk::KHRShaderDrawParametersExtensionName,
        vk::KHRCreateRenderpass2ExtensionName,
        vk::KHRSynchronization2ExtensionName,
        vk::KHRSwapchainExtensionName,
        vk::KHRSpirv14ExtensionName,
    };

    static void InspectDevice(vk::raii::PhysicalDevice &device)
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

    static bool IsDeviceSuitable(vk::raii::PhysicalDevice device)
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

    static vk::raii::PhysicalDevice
    PickPhysicalDevice(vk::raii::Instance &instance)
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

    static u32 FindGraphicsQueueFamily(vk::raii::PhysicalDevice &device,
                                       vk::SurfaceKHR surface)
    {
        auto familyProperties = device.getQueueFamilyProperties();

        for (u32 i = 0; i < familyProperties.size(); i++)
        {
            if (!(familyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics))
                continue;

            auto [result, presentSupported] =
                device.getSurfaceSupportKHR(i, surface);
            if (result != vk::Result::eSuccess)
                Panic("failed to query surface support for queue family");

            if (presentSupported == vk::True)
                return i;
        }
        Panic("no queue family supports both graphics and presentation to the "
              "given surface");
    }

    static vk::raii::Device
    CreateLogicalDevice(vk::raii::PhysicalDevice physicalDevice,
                        u32 graphicsIndex)
    {
        using namespace vk;

        std::array queuePriorities = {0.5f};
        DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.setQueueFamilyIndex(graphicsIndex);
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

    static vk::raii::CommandPool CreateCommandPool(vk::raii::Device &vkDevice,
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

    Device::Device(vk::raii::Instance &instance, vk::SurfaceKHR surface)
    {
        vkPhysicalDevice = PickPhysicalDevice(instance);
#if _DEBUG
        InspectDevice(vkPhysicalDevice);
#endif
        auto graphicsIndex = FindGraphicsQueueFamily(vkPhysicalDevice, surface);

        vkDevice = CreateLogicalDevice(vkPhysicalDevice, graphicsIndex);
        vkCommandPool = CreateCommandPool(vkDevice, graphicsIndex);
        vkGraphicsQueue = vkDevice.getQueue(graphicsIndex, 0);
        vkPresentQueue = vkGraphicsQueue;
    }

    /** Gets information about the surface support for the physical device */
    SurfaceSupport Device::QuerySurfaceSupport(vk::SurfaceKHR surface)
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

    void Device::ResetFence(vk::Fence fence)
    {
        vkDevice.resetFences(fence);
    }

    vk::Result Device::WaitForFence(vk::Fence fence)
    {
        return vkDevice.waitForFences(fence, vk::True, UINT64_MAX);
    }

    std::vector<vk::raii::CommandBuffer>
    Device::AllocateCommandBuffers(u32 count) const
    {
        vk::CommandBufferAllocateInfo allocateInfo{
            .commandPool = vkCommandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = count,
        };
        return std::move(*vkDevice.allocateCommandBuffers(allocateInfo));
    }

    vk::raii::Semaphore Device::CreateSemaphore() const
    {
        return std::move(*vkDevice.createSemaphore({}));
    }

    vk::raii::Fence Device::CreateFence(bool signaled) const
    {
        return std::move(*vkDevice.createFence({
            .flags = signaled ? vk::FenceCreateFlagBits::eSignaled
                              : vk::FenceCreateFlags{},
        }));
    }

    vk::raii::ShaderModule
    Device::CreateShaderModule(std::span<const u8> code) const
    {
        vk::ShaderModuleCreateInfo createInfo{
            .codeSize = code.size(),
            .pCode = reinterpret_cast<const u32 *>(code.data()),
        };
        return std::move(*vkDevice.createShaderModule(createInfo));
    }
} // namespace gd::rhi::vulkan
