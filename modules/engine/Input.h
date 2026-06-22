#pragma once
#include <SDL3/SDL.h>

namespace input
{
	struct MouseVector
	{
		float x;
		float y;
	};

	void update();
	bool isKeyPressed(SDL_Scancode scancode);
	bool wasKeyPressed(SDL_Scancode scancode);
	bool isMouseButtonPressed(int button);
	bool wasMouseButtonPressed(int button);
	bool minimized();
	void clear();
	bool shouldQuit();
}
