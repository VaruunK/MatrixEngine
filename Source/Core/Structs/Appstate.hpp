#pragma once

#include "Core/MatrixAPI.hpp"
#include <filesystem>

struct SDL_GPUDevice;
struct SDL_Window;

struct MATRIX_API Appstate {
	SDL_GPUDevice* device = nullptr;
	SDL_Window* window = nullptr;
	std::filesystem::path gamePath;
};