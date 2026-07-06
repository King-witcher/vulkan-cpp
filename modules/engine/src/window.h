#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "rust_types.h"

#include <SDL3/SDL.h>
#include <span>

namespace gd
{
	class Window
	{
	public:
		Window(const char *title);

		~Window();

		vk::raii::SurfaceKHR VulkanSurface(vk::raii::Instance &instance) const;
		std::span<const char *const> RequiredVulkanExtensions() const;
		vk::Extent2D Extent() const;
		void SetPosition(i32 x, i32 y) { SDL_SetWindowPosition(window, x, y); }

	private:
		SDL_Window *window;
	};
}
