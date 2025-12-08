#include "Input.h"
using namespace input;

bool shouldQuit_ = false;

const int KEY_COUNT = 512;
const int MOUSE_BUTTON_COUNT = 8;
bool keysDown[KEY_COUNT] = { false };
bool keysPressed[KEY_COUNT] = { false };
bool mouseButtonsDown[MOUSE_BUTTON_COUNT] = { false };
bool mouseButtonsPressed[MOUSE_BUTTON_COUNT] = { false };

MouseVector mousePos = { 0.0f, 0.0f };
MouseVector mouseRel = { 0.0f, 0.0f };

void input::update() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_EVENT_KEY_DOWN: {
			auto scancode = event.key.scancode;
			keysDown[scancode] = true;
			keysPressed[scancode] = true;
			break;
		}
		case SDL_EVENT_KEY_UP: {
			auto scancode = event.key.scancode;
			keysDown[scancode] = false;
			break;
		}
		case SDL_EVENT_MOUSE_BUTTON_DOWN: {
			auto button = event.button.button;
			if (button >= MOUSE_BUTTON_COUNT) break;
			mouseButtonsDown[button] = true;
			mouseButtonsPressed[button] = true;
			break;
		}
		case SDL_EVENT_MOUSE_BUTTON_UP: {
			auto button = event.button.button;
			if (button >= MOUSE_BUTTON_COUNT) break;
			mouseButtonsDown[button] = false;
			break;
		}
		case SDL_EVENT_QUIT: {
			shouldQuit_ = true;
			break;
		}
		case SDL_EVENT_MOUSE_MOTION: {
			mousePos.x = event.motion.x;
			mousePos.y = event.motion.y;
			mouseRel.x = event.motion.xrel;
			mouseRel.y = event.motion.yrel;
			break;
		}
		}
	}
}

bool input::isKeyPressed(SDL_Scancode scancode) {
	return keysDown[scancode];
}

bool input::wasKeyPressed(SDL_Scancode scancode) {
	return keysPressed[scancode];
}

bool input::isMouseButtonPressed(int button) {
	return mouseButtonsDown[button];
}

bool input::wasMouseButtonPressed(int button) {
	return mouseButtonsPressed[button];
	return false;
}

void input::clear() {
	for (int i = 0; i < KEY_COUNT; i++) {
		keysPressed[i] = false;
		keysDown[i] = false;
	}
	for (int i = 0; i < MOUSE_BUTTON_COUNT; i++) {
		mouseButtonsPressed[i] = false;
		mouseButtonsDown[i] = false;
	}
}

bool input::shouldQuit() {
	return shouldQuit_;
}
