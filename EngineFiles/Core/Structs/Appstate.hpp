#pragma once

#include "Core/MatrixAPI.hpp"

struct SDL_GPUDevice;
struct SDL_Window;

struct MATRIX_API Appstate {
	SDL_GPUDevice* device;
	SDL_Window* window;
};