#include <SDL3/SDL.h>

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

    Window::Size Window::Extent() const
    {
        i32 width, height;
        SDL_GetWindowSizeInPixels(window, &width, &height);
        return {static_cast<u32>(width), static_cast<u32>(height)};
    }

    void Window::SetPosition(i32 x, i32 y)
    {
        SDL_SetWindowPosition(window, x, y);
    }
} // namespace gd
