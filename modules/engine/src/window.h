#pragma once

#include "rust_types.h"

struct SDL_Window;

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
        ~Window();

        // Handle cru para o backend de RHI criar a surface/device.
        SDL_Window *SdlHandle() const { return window; }
        Size Extent() const;
        void SetPosition(i32 x, i32 y);

    private:
        SDL_Window *window;
    };
} // namespace gd
