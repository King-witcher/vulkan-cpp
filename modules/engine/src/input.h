#pragma once

#include <SDL3/SDL.h>

namespace gd
{
    struct MouseVector
    {
        float x;
        float y;
    };

    class Input
    {

    public:
        Input() {}
        Input(Input &) = delete;
        Input &operator=(Input &) = delete;

        bool IsKeyDown(SDL_Scancode scancode);
        bool WasKeyPressed(SDL_Scancode scancode);

        bool IsMouseBtnDown(int button);
        bool WasMouseBtnPressed(int button);

        bool Minimized() { return minimized; }
        bool ShouldQuit() { return shouldQuit; }

        void Update();
        void Clear();

    private:
        static const int SDL_KEY_COUNT = 512;
        static const int MOUSE_BUTTON_COUNT = 8;

        bool keysDown[SDL_KEY_COUNT] = {};
        bool keysPressed[SDL_KEY_COUNT] = {};

        bool mouseButtonsDown[MOUSE_BUTTON_COUNT] = {};
        bool mouseButtonsPressed[MOUSE_BUTTON_COUNT] = {};

        bool minimized = false;
        bool shouldQuit = false;

        MouseVector mouseAbsolute = {0.0f, 0.0f};
        MouseVector mouseDelta = {0.0f, 0.0f};
    };
} // namespace gd
