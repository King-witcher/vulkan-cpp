#include "Device.h"
#include "RustTypes.h"
#include <vector>

using namespace vkwiz;

std::vector<const char*> REQUIRED_EXTENSIONS = {
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
		{
			.synchronization2 = true,
			.dynamicRendering = true,
		},      // Enable dynamic rendering from Vulkan 1.3
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

vkwiz::Device::Device(vk::raii::Instance& instance, vk::raii::SurfaceKHR& surface) : vkSurface(surface)
{
	vkPhysicalDevice_ = pickPhysicalDevice(instance);
	graphicsIndex_ = findQueueFamilies(vkPhysicalDevice_);

	// TODO: Consider different queue families for presentation
	if (!vkPhysicalDevice_.getSurfaceSupportKHR(graphicsIndex_, this->vkSurface))
		throw std::runtime_error("Selected physical device does not support presentation to the given surface.");

	vkDevice_ = createLogicalDevice(vkPhysicalDevice_, graphicsIndex_);
	vkGraphicsQueue_ = vk::raii::Queue(vkDevice_, graphicsIndex_, 0);
	vkPresentQueue_ = vkGraphicsQueue_;

	vkCommandPool_ = vk::raii::CommandPool(
		vkDevice_,
		vk::CommandPoolCreateInfo{
			.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			.queueFamilyIndex = graphicsIndex_,
		}
		);
}

SwapchainSurfaceSupportDetails vkwiz::Device::querySwapchainSupportDetails(vk::raii::SurfaceKHR& surface, vk::Extent2D windowExtent)
{
	auto capabilities = vkPhysicalDevice_.getSurfaceCapabilitiesKHR(surface);
	auto formats = vkPhysicalDevice_.getSurfaceFormatsKHR(surface);
	auto presentModes = vkPhysicalDevice_.getSurfacePresentModesKHR(surface);

	return SwapchainSurfaceSupportDetails{
		.capabilities = capabilities,
		.formats = formats,
		.presentModes = presentModes,
	};
}

vk::raii::Device& vkwiz::Device::vkDevice()
{
	return vkDevice_;
}

u32 vkwiz::Device::graphicsIndex()
{
	return graphicsIndex_;
}

void vkwiz::Device::resetFence(vk::Fence fence)
{
	vkDevice_.resetFences(fence);
}

vk::Result vkwiz::Device::waitForFence(vk::Fence fence)
{
	return vkDevice_.waitForFences(fence, vk::True, UINT64_MAX);
}

void vkwiz::Device::submitGraphics(vk::SubmitInfo submitInfo, vk::Fence fence)
{
	vkGraphicsQueue_.submit(submitInfo, fence);
}

void vkwiz::Device::present(vk::PresentInfoKHR presentInfo)
{
	if (vkPresentQueue_.presentKHR(presentInfo) != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to present swapchain image.");
	}
}

std::vector<vk::raii::CommandBuffer> vkwiz::Device::allocateCommandBuffers(u32 count) const
{
	vk::CommandBufferAllocateInfo allocateInfo{
		.commandPool = vkCommandPool_,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = count,
	};
	return vkDevice_.allocateCommandBuffers(allocateInfo);
}
