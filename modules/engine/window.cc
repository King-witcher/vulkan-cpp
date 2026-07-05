#include "window.h"
#include "panic.h"
#include <SDL3/SDL_vulkan.h>

gd::Window::Window(const char *title)
{
	window = SDL_CreateWindow(title, 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_MOUSE_RELATIVE_MODE | SDL_WINDOW_RESIZABLE);
}

gd::Window::~Window()
{
	SDL_DestroyWindow(window);
}

vk::raii::SurfaceKHR gd::Window::VulkanSurface(vk::raii::Instance &instance) const
{
	VkSurfaceKHR surface;
	if (!SDL_Vulkan_CreateSurface(window, *instance, nullptr, &surface))
	{
		Panic("Failed to create Vulkan surface.");
	}
	return vk::raii::SurfaceKHR(instance, surface);
}

std::span<const char *const> gd::Window::RequiredVulkanExtensions() const
{
	u32 extensionCount = 0;
	auto extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
	return std::span(extensions, extensionCount);
}

vk::Extent2D gd::Window::Extent() const
{
	i32 width, height;
	SDL_GetWindowSizeInPixels(window, &width, &height);
	return {
		.width = static_cast<u32>(width),
		.height = static_cast<u32>(height),
	};
}
