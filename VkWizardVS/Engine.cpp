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

#ifdef NDEBUG
	auto layers = std::vector<const char*>{};
#else
	auto layers = std::vector<const char*>{ "VK_LAYER_KHRONOS_validation" };
	std::cout << "Enabling validation layers..." << std::endl;
#endif

	vk::InstanceCreateInfo createInfo{
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<u32>(layers.size()),
		.ppEnabledLayerNames = layers.data(),
		.enabledExtensionCount = static_cast<u32>(extensionCount),
		.ppEnabledExtensionNames = requiredExtensions,
	};

	auto instance = vk::raii::Instance(vkContext, createInfo);
	std::cout << "Vulkan instance created." << std::endl;
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


vkwiz::Engine::Engine() {
	window = createWindow(800, 600, "VkWizardVS");
	auto windowExtent = window->getExtent();
	vkInstance = createInstance();
	surface = window->getVulkanSurface(vkInstance);
	device = createDevice(vkInstance, surface);
	swapChain = createSwapChain(*device, surface, windowExtent);
	pipeline = std::make_unique<Pipeline>(*device, *swapChain, "shaders/shader.spv", windowExtent);
}