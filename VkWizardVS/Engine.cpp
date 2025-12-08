#include "Engine.h"
#include "RustTypes.h"
#include "Input.h"
#include <iostream>

using namespace vkwiz;

void vkwiz::Engine::run() {
	for (;;) {
		input::update();
		if (input::shouldQuit()) break;
	}
	std::cout << "Exiting engine loop." << std::endl;
}

vk::raii::Instance vkwiz::Engine::createInstance() {
	constexpr vk::ApplicationInfo appInfo{
		.pApplicationName = "VkWizard",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = vk::ApiVersion14
	};

	u32 extensionCount = 0;
	auto requiredExtensions = window->getRequiredVulkanExtensions(&extensionCount);
	auto supportedExtensions = vk::enumerateInstanceExtensionProperties();
	// TODO: check for supported extensions

#ifdef _DEBUG
	auto layers = std::vector<const char*>{ "VK_LAYER_KHRONOS_validation" };
	std::cout << "Enabling validation layers..." << std::endl;
#else
	auto layers = std::vector<const char*>{};
#endif

	vk::InstanceCreateInfo createInfo{
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<u32>(layers.size()),
		.ppEnabledLayerNames = layers.data(),
		.enabledExtensionCount = static_cast<u32>(extensionCount),
		.ppEnabledExtensionNames = requiredExtensions,
	};

	auto instance = vk::raii::Instance(vkContext, createInfo);
	std::cout << "Vulkan instance created successfully." << std::endl;
	return instance;
}

std::unique_ptr<Window> createWindow(u32 width, u32 height, const char* title) {
	return std::make_unique<Window>(title);
}

std::unique_ptr<Device> createDevice(vk::raii::Instance& instance, vk::raii::SurfaceKHR& surface) {
	return std::make_unique<Device>(instance, surface);
}

std::unique_ptr<SwapChain> createSwapChain(Device& device, vk::raii::SurfaceKHR& surface, vk::Extent2D windowExtent) {
	return std::make_unique<SwapChain>(device, surface, windowExtent);
}

std::unique_ptr<Pipeline> createPipeline(Device& device, const std::string& shaderPath) {
	return std::make_unique<Pipeline>(device, shaderPath);
}


vkwiz::Engine::Engine() {
	window = createWindow(800, 600, "VkWizardVS");
	vkInstance = createInstance();
	surface = window->getVulkanSurface(vkInstance);
	device = createDevice(vkInstance, surface);
	swapChain = createSwapChain(*device, surface, vk::Extent2D{ 800, 600 });
	pipeline = createPipeline(*device, "shaders/shader.spv");
}