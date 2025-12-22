#include "Device.h"
#include "RustTypes.h"
#include "Panic.h"

#include <vector>
#include <array>
#include <iostream>

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

u32 findQueueFamilies(vk::raii::PhysicalDevice device)
{
	auto queueFamilies = device.getQueueFamilyProperties();
	auto familyProperty = std::find_if(
			queueFamilies.begin(),
			queueFamilies.end(),
			[](vk::QueueFamilyProperties const &properties)
			{
				return properties.queueFlags & vk::QueueFlagBits::eGraphics;
			});
	return static_cast<u32>(std::distance(queueFamilies.begin(), familyProperty));
}

vk::raii::Device createLogicalDevice(vk::raii::PhysicalDevice physicalDevice, u32 graphicsIndex)
{
	std::array queuePriorities = {0.5f};
	std::array queueCreateInfos = {vk::DeviceQueueCreateInfo()};
	queueCreateInfos[0].setQueuePriorities(queuePriorities);

	vk::StructureChain<
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
			featureChain = {
					{}, // vk::PhysicalDeviceFeatures2 (empty for now)
					{
							.synchronization2 = true,
							.dynamicRendering = true,
					},														 // Enable dynamic rendering from Vulkan 1.3
					{.extendedDynamicState = true} // Enable extended dynamic state from the extension
			};

	vk::DeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.setPNext(&featureChain.get());
	deviceCreateInfo.setQueueCreateInfos(queueCreateInfos);
	deviceCreateInfo.setPEnabledExtensionNames(REQUIRED_EXTENSIONS);

	return std::move(*physicalDevice.createDevice(deviceCreateInfo));
}

vkwiz::Device::Device(vk::raii::Instance &instance, vk::raii::SurfaceKHR &surface)
{
	vkPhysicalDevice_ = pickPhysicalDevice(instance);
	graphicsIndex_ = findQueueFamilies(vkPhysicalDevice_);

	// TODO: Consider different queue families for presentation
	if (!*(vkPhysicalDevice_.getSurfaceSupportKHR(graphicsIndex_, surface)))
		throw std::runtime_error("Selected physical device does not support presentation to the given surface.");

	vkDevice_ = createLogicalDevice(vkPhysicalDevice_, graphicsIndex_);
	vkGraphicsQueue_ = std::move(vkDevice_.getQueue(graphicsIndex_, 0));
	vkPresentQueue_ = vkGraphicsQueue_;

	vkCommandPool_ = std::move(*vkDevice_.createCommandPool(
			vk::CommandPoolCreateInfo{
					.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
					.queueFamilyIndex = graphicsIndex_,
			}));
}

SwapchainSurfaceSupportDetails vkwiz::Device::querySwapchainSupportDetails(vk::raii::SurfaceKHR &surface)
{
	auto capabilities = *vkPhysicalDevice_.getSurfaceCapabilitiesKHR(surface);
	auto formats = *vkPhysicalDevice_.getSurfaceFormatsKHR(surface);
	auto presentModes = *vkPhysicalDevice_.getSurfacePresentModesKHR(surface);

	return SwapchainSurfaceSupportDetails{
			.capabilities = capabilities,
			.formats = formats,
			.presentModes = presentModes,
	};
}

u32 vkwiz::Device::graphicsIndex()
{
	return graphicsIndex_;
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
