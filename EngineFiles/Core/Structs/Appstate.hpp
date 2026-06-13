#pragma once

struct SDL_GPUDevice;
struct SDL_Window;

struct Appstate {
	SDL_GPUDevice* device;
	SDL_Window* window;
};