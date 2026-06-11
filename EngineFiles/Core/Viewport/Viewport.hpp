#pragma once

#include "Core/Viewport/ViewportController/ViewportController.hpp"
#include "Core/Viewport/ViewportRenderer/ViewportRenderer.hpp"
#include "Core/Structs/FrameData.hpp"

struct View;
struct SDL_GPUDevice;
struct SDL_Window;

class WorldRenderer;

class Viewport {
public:
	Viewport(SDL_GPUDevice* device, SDL_Window* window);
	~Viewport() = default;

	void Initialize(WorldRenderer* worldRenderer);
	void Tick(float deltaTime);
	void Render();

	void SetCameraSpeed(int& speed);

	const View& GetCameraView() const;

	float deltaSeconds = 0.0f;
private:

	SDL_GPUDevice* device = nullptr;
	SDL_Window* window = nullptr;

	ViewportController controller;
	ViewportRenderer renderer;

	WorldRenderer* worldRenderer = nullptr;
};