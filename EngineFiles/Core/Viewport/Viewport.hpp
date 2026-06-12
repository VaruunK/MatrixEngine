#pragma once

#include "Core/Viewport/ViewportController/ViewportController.hpp"
#include "Core/Viewport/ViewportRenderer/ViewportRenderer.hpp"
#include "Core/Viewport/ViewportCamera/ViewportCamera.hpp"
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

	void GetEntityAtPixelLocation(float x, float y);

	float deltaSeconds = 0.0f;
private:
	const View& GetCameraView() const;
	SDL_GPUDevice* device = nullptr;
	SDL_Window* window = nullptr;

	ViewportCamera camera;
	ViewportController controller;
	ViewportRenderer renderer;

	SDL_GPUTexture* offscreenTexture = nullptr;
	SDL_GPUSampler* offscreenSampler = nullptr;

	WorldRenderer* worldRenderer = nullptr;
};