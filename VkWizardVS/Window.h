#pragma once

#include "Vulkan.h"
#include "RustTypes.h"
#include <SDL3/SDL.h>

namespace vkwiz {
	class Window {
	public:
		Window(const char* title);

		~Window();

		vk::raii::SurfaceKHR getVulkanSurface(vk::raii::Instance& instance) const;
		char const* const* getRequiredVulkanExtensions(u32* extensionCount) const;
		vk::Extent2D getExtent() const;

	private:
		SDL_Window* window_;
	};
}