#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "panic.h"

#include "window.h"

namespace gd
{

    Window::Window(const char *title)
    {
        window = SDL_CreateWindow(title, 800, 600,
                                  SDL_WINDOW_VULKAN |
                                      SDL_WINDOW_MOUSE_RELATIVE_MODE |
                                      SDL_WINDOW_RESIZABLE);
    }

    Window::~Window()
    {
        SDL_DestroyWindow(window);
    }

    VkSurfaceKHR Window::VulkanSurface(VkInstance instance) const
    {
        VkSurfaceKHR surface;
        if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface))
        {
            Panic("Failed to create Vulkan surface.");
        }
        return surface;
    }

    std::span<const char *const> Window::RequiredVulkanExtensions() const
    {
        u32 extensionCount = 0;
        auto extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
        return std::span(extensions, extensionCount);
    }

    void Window::SetPosition(i32 x, i32 y)
    {
        SDL_SetWindowPosition(window, x, y);
    }

    Window::Size Window::GetSize() const
    {
        i32 width, height;
        SDL_GetWindowSizeInPixels(window, &width, &height);
        return {
            .width = static_cast<u32>(width),
            .height = static_cast<u32>(height),
        };
    }
} // namespace gd
