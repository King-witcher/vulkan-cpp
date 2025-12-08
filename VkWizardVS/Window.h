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
		void getDrawableSize(i32* width, i32* height) const;

	private:
		SDL_Window* window_;
	};
}