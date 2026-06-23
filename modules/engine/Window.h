#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "RustTypes.h"

#include <SDL3/SDL.h>
#include <span>

namespace vkwiz
{
	class Window
	{
	public:
		Window(const char *title);

		~Window();

		vk::raii::SurfaceKHR getVulkanSurface(vk::raii::Instance &instance) const;
		std::span<const char *const> getRequiredVulkanExtensions() const;
		vk::Extent2D extent() const;
		void setPosition(i32 x, i32 y) { SDL_SetWindowPosition(window_, x, y); }

	private:
		SDL_Window *window_;
	};
}
