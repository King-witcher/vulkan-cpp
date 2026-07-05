#include "Input.h"

void gd::Input::Update()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_KEY_DOWN:
        {
            auto scancode = event.key.scancode;
            keysDown[scancode] = true;
            keysPressed[scancode] = true;
            break;
        }
        case SDL_EVENT_KEY_UP:
        {
            auto scancode = event.key.scancode;
            keysDown[scancode] = false;
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            auto button = event.button.button;
            if (button >= MOUSE_BUTTON_COUNT)
                break;
            mouseButtonsDown[button] = true;
            mouseButtonsPressed[button] = true;
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            auto button = event.button.button;
            if (button >= MOUSE_BUTTON_COUNT)
                break;
            mouseButtonsDown[button] = false;
            break;
        }
        case SDL_EVENT_QUIT:
        {
            shouldQuit = true;
            break;
        }
        case SDL_EVENT_MOUSE_MOTION:
        {
            mouseAbsolute.x = event.motion.x;
            mouseAbsolute.y = event.motion.y;
            mouseDelta.x = event.motion.xrel;
            mouseDelta.y = event.motion.yrel;
            break;
        }
        case SDL_EVENT_WINDOW_MINIMIZED:
        {
            minimized = true;
            break;
        }
        case SDL_EVENT_WINDOW_RESTORED:
        {
            minimized = false;
            break;
        }
        }
    }
}

bool gd::Input::IsKeyDown(SDL_Scancode scancode)
{
    return keysDown[scancode];
}

bool gd::Input::WasKeyPressed(SDL_Scancode scancode)
{
    return keysPressed[scancode];
}

bool gd::Input::IsMouseBtnDown(int button)
{
    return mouseButtonsDown[button];
}

bool gd::Input::WasMouseBtnPressed(int button)
{
    return mouseButtonsPressed[button];
    return false;
}

void gd::Input::Clear()
{
    for (int i = 0; i < SDL_KEY_COUNT; i++)
    {
        keysPressed[i] = false;
        keysDown[i] = false;
    }
    for (int i = 0; i < MOUSE_BUTTON_COUNT; i++)
    {
        mouseButtonsPressed[i] = false;
        mouseButtonsDown[i] = false;
    }
}
