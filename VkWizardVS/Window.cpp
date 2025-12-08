#include "Window.h"
#include <SDL3/SDL_vulkan.h>

vkwiz::Window::Window(const char* title) {
	window_ = SDL_CreateWindow(title, 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_MOUSE_RELATIVE_MODE);
}

vkwiz::Window::~Window() {
	SDL_DestroyWindow(window_);
}

vk::raii::SurfaceKHR vkwiz::Window::getVulkanSurface(vk::raii::Instance& instance) const
{
	VkSurfaceKHR surface;
	if (!SDL_Vulkan_CreateSurface(window_, *instance, nullptr, &surface)) {
		throw std::runtime_error("Failed to create Vulkan surface.");
	}
	return vk::raii::SurfaceKHR(instance, surface);
}

char const* const* vkwiz::Window::getRequiredVulkanExtensions(u32* extensionCount) const {
	return SDL_Vulkan_GetInstanceExtensions(extensionCount);
}

void vkwiz::Window::getDrawableSize(i32* width, i32* height) const
{
	SDL_GetWindowSizeInPixels(window_, width, height);
}
