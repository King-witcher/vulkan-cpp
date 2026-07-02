#include "Device.h"
#include "RustTypes.h"
#include "Panic.h"

#include <vector>
#include <array>
#include <iostream>
#include <format>

using namespace vkwiz;

std::array REQUIRED_EXTENSIONS = {
	vk::KHRShaderDrawParametersExtensionName,
	vk::KHRCreateRenderpass2ExtensionName,
	vk::KHRSynchronization2ExtensionName,
	vk::KHRSwapchainExtensionName,
	vk::KHRSpirv14ExtensionName,
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

vk::raii::PhysicalDevice pickPhysicalDevice(vk::raii::Instance &instance)
{
	auto [result, devices] = instance.enumeratePhysicalDevices();
	if (result != vk::Result::eSuccess)
		throw std::runtime_error("failed to enumerate physical devices");
	if (devices.size() == 0)
		throw std::runtime_error("no vulkan compatible GPU found");

	// TODO: Pick the most suitable device
	for (const auto &device : devices)
	{
		if (isDeviceSuitable(device))
			return device;
	}
	throw std::runtime_error("failed to find a suitable GPU!");
}

/** Gets the index of the first queue family that supports graphics in a specific device. */
u32 findGraphicsQueueFamily(vk::raii::PhysicalDevice device)
{
	auto familyProperties = device.getQueueFamilyProperties();

	for (u32 i = 0; i < familyProperties.size(); i++)
	{
		auto familyProperty = familyProperties[i];
		if (familyProperty.queueFlags & vk::QueueFlagBits::eGraphics)
			return i;
	}
	throw new std::exception("unreachable: Vulkan requires implementations to expose at least one graphics queue family");
}

vk::raii::Device createLogicalDevice(vk::raii::PhysicalDevice physicalDevice, u32 graphicsIndex)
{
	using namespace vk;

	std::array queuePriorities = {0.5f};
	DeviceQueueCreateInfo queueCreateInfo;
	queueCreateInfo.setQueuePriorities(queuePriorities);
	std::array queueCreateInfos = {queueCreateInfo};

	StructureChain<PhysicalDeviceFeatures2, PhysicalDeviceVulkan13Features, PhysicalDeviceExtendedDynamicStateFeaturesEXT>
		featureChain = {
			{}, // vk::PhysicalDeviceFeatures2 (empty for now)
			{
				.synchronization2 = true,
				.dynamicRendering = true,
			},							   // Enable dynamic rendering from Vulkan 1.3
			{.extendedDynamicState = true} // Enable extended dynamic state from the extension
		};

	DeviceCreateInfo deviceInfo{};
	deviceInfo.setPNext(&featureChain.get());
	deviceInfo.setQueueCreateInfos(queueCreateInfos);
	deviceInfo.setPEnabledExtensionNames(REQUIRED_EXTENSIONS);

	auto createResult = physicalDevice.createDevice(deviceInfo);
	if (!createResult.has_value())
	{
		panic("failed to create device");
	}

	return std::move(*createResult);
}

vk::raii::CommandPool createCommandPool(vk::raii::Device &vkDevice, u32 graphicsIndex)
{
	using namespace vk;
	CommandPoolCreateInfo createInfo;
	createInfo.setFlags(CommandPoolCreateFlagBits::eResetCommandBuffer);
	createInfo.setQueueFamilyIndex(graphicsIndex);

	auto createResult = vkDevice.createCommandPool(createInfo);
	if (createResult.has_value())
		return std::move(*createResult);
	panic("failed to create command pool");
}

vkwiz::Device::Device(vk::raii::Instance &instance, vk::raii::SurfaceKHR &surface)
{
	vkPhysicalDevice_ = pickPhysicalDevice(instance);
	auto graphicsIndex = findGraphicsQueueFamily(vkPhysicalDevice_);

	// TODO: Consider different queue families for presentation
	auto [surfaceSupportResult, surfaceSupport] = vkPhysicalDevice_.getSurfaceSupportKHR(graphicsIndex, surface);
	if (surfaceSupportResult != vk::Result::eSuccess)
		panic("Failed to get surface support for physical device.");
	if (surfaceSupport == vk::False)
		panic("Selected physical device does not support presentation to the given surface.");

	vkDevice_ = createLogicalDevice(vkPhysicalDevice_, graphicsIndex);
	vkCommandPool_ = createCommandPool(vkDevice_, graphicsIndex);
	vkGraphicsQueue_ = vkDevice_.getQueue(graphicsIndex, 0);
	vkPresentQueue_ = vkGraphicsQueue_;
}

/** Gets information about the surface support for the physical device */
SurfaceSupport vkwiz::Device::getSurfaceSupport(vk::raii::SurfaceKHR &surface)
{
	auto capabilities = vkPhysicalDevice_.getSurfaceCapabilitiesKHR(surface);
	auto formats = vkPhysicalDevice_.getSurfaceFormatsKHR(surface);
	auto presentModes = vkPhysicalDevice_.getSurfacePresentModesKHR(surface);

	if (capabilities.result != vk::Result::eSuccess)
		panic("Failed to get surface capabilities for physical device.");
	if (formats.result != vk::Result::eSuccess)
		panic("Failed to get surface formats for physical device.");
	if (presentModes.result != vk::Result::eSuccess)
		panic("Failed to get surface present modes for physical device.");

	return SurfaceSupport{
		.capabilities = *capabilities,
		.formats = *formats,
		.presentModes = *presentModes,
	};
}

void vkwiz::Device::resetFence(vk::raii::Fence &fence)
{
	vkDevice_.resetFences(*fence);
}

vk::Result vkwiz::Device::waitForFence(vk::raii::Fence &fence)
{
	return vkDevice_.waitForFences(*fence, vk::True, UINT64_MAX);
}

void vkwiz::Device::submitGraphics(vk::SubmitInfo submitInfo, vk::Fence fence)
{
	vkGraphicsQueue_.submit(submitInfo, fence);
}

bool vkwiz::Device::present(vk::PresentInfoKHR &presentInfo)
{
	auto result = vkPresentQueue_.presentKHR(presentInfo);
	switch (result)
	{
	case vk::Result::eSuccess:
		return true;
	case vk::Result::eErrorOutOfDateKHR:
		std::cerr << "swap chain was out of date when presenting" << std::endl;
		return false;
	case vk::Result::eSuboptimalKHR:
		std::cerr << "swap chain was suboptimal when presenting" << std::endl;
		return false;
	default:
		panic("failed to present swapchain image");
	}
}

std::vector<vk::raii::CommandBuffer> vkwiz::Device::allocateCommandBuffers(u32 count) const
{
	vk::CommandBufferAllocateInfo allocateInfo{
		.commandPool = vkCommandPool_,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = count,
	};
	return std::move(*vkDevice_.allocateCommandBuffers(allocateInfo));
}

vk::raii::Semaphore vkwiz::Device::createSemaphore() const
{
	return std::move(*vkDevice_.createSemaphore({}));
}

vk::raii::Fence vkwiz::Device::createFence(bool signaled) const
{
	return std::move(*vkDevice_.createFence({
		.flags = signaled ? vk::FenceCreateFlagBits::eSignaled : vk::FenceCreateFlags{},
	}));
}

vk::raii::ShaderModule vkwiz::Device::createShaderModule(const std::vector<u8> code) const
{
	vk::ShaderModuleCreateInfo createInfo{
		.codeSize = code.size(),
		.pCode = reinterpret_cast<const u32 *>(code.data()),
	};
	return std::move(*vkDevice_.createShaderModule(createInfo));
}

std::tuple<vk::raii::Buffer, vk::raii::DeviceMemory> vkwiz::Device::alloc(usize size)
{
	// Create buffer
	vk::BufferCreateInfo bufferInfo;
	bufferInfo.setSize(size);
	bufferInfo.setUsage(vk::BufferUsageFlagBits::eVertexBuffer);
	bufferInfo.setSharingMode(vk::SharingMode::eExclusive);
	auto bufferResult = vkDevice_.createBuffer(bufferInfo);
	if (!bufferResult.has_value())
		panic("failed to create buffer");
	auto buffer = std::move(*bufferResult);

	// Allocate memory
	auto requirements = buffer.getMemoryRequirements();
	// Estamos usando HostCoherent para não precisar dar vkDevice_.mapFlushedMemoryRanges() depois de escrever na memória mapeada e vkDevice_.invalidateMappedMemoryRanges antes de ler da memória mapeada.
	// Mas isso tem desempenho pior e pode ser mudado depois.
	auto memType = findMemoryType(requirements.memoryTypeBits,
								  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	vk::MemoryAllocateInfo memInfo;
	memInfo.setAllocationSize(requirements.size);
	memInfo.setMemoryTypeIndex(memType);
	auto memResult = vkDevice_.allocateMemory(memInfo);
	if (!memResult.has_value())
		panic("failed to alloc memory for buffer");
	auto memory = std::move(*memResult);

	// Vincula a memória alocada ao buffer. Sem isso, o buffer não tem
	// armazenamento e qualquer uso dele dispara VUID-...-pBuffers-00628.
	buffer.bindMemory(*memory, 0);

	return std::make_tuple(std::move(buffer), std::move(memory));
}

// Existem heaps diferentes como VRAM e espaço de swap na RAM pra quando a VRAM acaba. São heaps diferentes.
// Dentro de cada heap, existem tipos diferentes de memória.
u32 vkwiz::Device::findMemoryType(u32 supportedTypes, vk::MemoryPropertyFlags properties)
{
	auto memProps = vkPhysicalDevice_.getMemoryProperties2();
	for (u32 i = 0; i < memProps.memoryProperties.memoryTypeCount; i++)
	{
		if ((supportedTypes & (1 << i)) &&														 // Buffer suporta tipo i?
			(memProps.memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) // Tipo i tem todas as flags que eu pedi?
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type");
}
