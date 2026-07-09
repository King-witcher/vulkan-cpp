#pragma once

#include <span>

#include "rust_types.h"

struct SDL_Window;
typedef struct VkInstance_T *VkInstance;
typedef struct VkSurfaceKHR_T *VkSurfaceKHR;

namespace gd
{
    class Window
    {
    public:
        struct Size
        {
            u32 width;
            u32 height;
        };

        Window(const char *title);

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        ~Window();

        VkSurfaceKHR VulkanSurface(VkInstance instance) const;
        std::span<const char *const> RequiredVulkanExtensions() const;

        Size GetSize() const;
        void SetPosition(i32 x, i32 y);

    private:
        SDL_Window *window;
    };
} // namespace gd
