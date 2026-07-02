#include "Window.h"
#include <SDL3/SDL_vulkan.h>

gd::Window::Window(const char *title)
{
	window_ = SDL_CreateWindow(title, 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_MOUSE_RELATIVE_MODE | SDL_WINDOW_RESIZABLE);
}

gd::Window::~Window()
{
	SDL_DestroyWindow(window_);
}

vk::raii::SurfaceKHR gd::Window::getVulkanSurface(vk::raii::Instance &instance) const
{
	VkSurfaceKHR surface;
	if (!SDL_Vulkan_CreateSurface(window_, *instance, nullptr, &surface))
	{
		throw std::runtime_error("Failed to create Vulkan surface.");
	}
	return vk::raii::SurfaceKHR(instance, surface);
}

std::span<const char *const> gd::Window::getRequiredVulkanExtensions() const
{
	u32 extensionCount = 0;
	auto extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
	return std::span(extensions, extensionCount);
}

vk::Extent2D gd::Window::extent() const
{
	i32 width, height;
	SDL_GetWindowSizeInPixels(window_, &width, &height);
	return {
		.width = static_cast<u32>(width),
		.height = static_cast<u32>(height),
	};
}
