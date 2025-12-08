#include "Device.h"
#include "RustTypes.h"
#include <vector>

using namespace vkwiz;

std::vector<const char*> REQUIRED_EXTENSIONS = {
	vk::KHRSwapchainExtensionName,
	vk::KHRSpirv14ExtensionName,
	vk::KHRSynchronization2ExtensionName,
	vk::KHRCreateRenderpass2ExtensionName,
};

bool isDeviceSuitable(vk::raii::PhysicalDevice device)
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

vk::raii::PhysicalDevice pickPhysicalDevice(vk::raii::Instance& instance)
{
	auto devices = instance.enumeratePhysicalDevices();
	if (devices.size() == 0)
		throw std::runtime_error("failed to find GPUs with Vulkan support!");

	// TODO: Pick the most suitable device
	for (const auto& device : devices)
	{
		if (isDeviceSuitable(device))
			return device;
	}
}

u32 findQueueFamilies(vk::raii::PhysicalDevice device)
{
	auto queueFamilies = device.getQueueFamilyProperties();
	auto familyProperty = std::find_if(
		queueFamilies.begin(),
		queueFamilies.end(),
		[](vk::QueueFamilyProperties const& properties) {
			return properties.queueFlags & vk::QueueFlagBits::eGraphics;
		}
	);
	return static_cast<u32>(std::distance(queueFamilies.begin(), familyProperty));
}

vk::raii::Device createLogicalDevice(vk::raii::PhysicalDevice physicalDevice, u32 graphicsIndex)
{
	f32 queuePriority = 0.5f;
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
		.queueFamilyIndex = graphicsIndex,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
	};

	vk::StructureChain<
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
	> featureChain = {
		{},                               // vk::PhysicalDeviceFeatures2 (empty for now)
		{.dynamicRendering = true },      // Enable dynamic rendering from Vulkan 1.3
		{.extendedDynamicState = true }   // Enable extended dynamic state from the extension
	};

	vk::DeviceCreateInfo deviceCreateInfo{
		.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &deviceQueueCreateInfo,
		.enabledExtensionCount = static_cast<u32>(REQUIRED_EXTENSIONS.size()),
		.ppEnabledExtensionNames = REQUIRED_EXTENSIONS.data(),
	};

	return vk::raii::Device(physicalDevice, deviceCreateInfo);
}

vkwiz::Device::Device(vk::raii::Instance& instance, vk::raii::SurfaceKHR& surface) : surface(surface)
{
	physicalDevice = pickPhysicalDevice(instance);
	auto graphicsIndex = findQueueFamilies(physicalDevice);

	// TODO: Consider different queue families for presentation
	if (!physicalDevice.getSurfaceSupportKHR(graphicsIndex, this->surface))
		throw std::runtime_error("Selected physical device does not support presentation to the given surface.");

	device = createLogicalDevice(physicalDevice, graphicsIndex);
	graphicsQueue = vk::raii::Queue(device, graphicsIndex, 0);
	presentQueue = graphicsQueue;
}

SwapchainSurfaceSupportDetails vkwiz::Device::querySwapchainSupportDetails(vk::raii::SurfaceKHR& surface, vk::Extent2D windowExtent)
{
	auto capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	auto formats = physicalDevice.getSurfaceFormatsKHR(surface);
	auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

	return SwapchainSurfaceSupportDetails{
		.capabilities = capabilities,
		.formats = formats,
		.presentModes = presentModes,
	};
}

vk::raii::Device& vkwiz::Device::getDevice()
{
	return device;
}
