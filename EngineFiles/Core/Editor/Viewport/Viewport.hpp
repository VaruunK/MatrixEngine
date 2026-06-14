#pragma once
#include "Core/Editor/Viewport/ViewportController/ViewportController.hpp"
#include "Core/Editor/Render/ViewportRenderer/ViewportRenderer.hpp"
#include "Core/Editor/Viewport/ViewportCamera/ViewportCamera.hpp"
#include "Core/Structs/FrameData.hpp"

struct View;
struct Appstate;
struct Frame;

class WorldRenderer;
class Editor;

class Viewport {
public:
	Viewport(Appstate& appstate, WorldRenderer* worldRenderer);
	~Viewport() = default;

	void Initialize();
	void Tick(float deltaTime);
	void Render(FrameData& frame);

	void SetCameraSpeed(int& speed);

	const View& GetCameraView() const;
	ViewportCamera& GetCamera() { return camera; }

	float deltaSeconds = 0.0f;

	bool clicked = false;
	float mouseClickX = -1.0f;
	float mouseClickY = -1.0f;
private:

	Appstate& appstate;

	ViewportCamera camera;
	ViewportController controller;
	ViewportRenderer viewportRenderer;

	SDL_GPUTexture* offscreenTexture = nullptr;
	SDL_GPUSampler* offscreenSampler = nullptr;

	WorldRenderer* worldRenderer = nullptr;
};