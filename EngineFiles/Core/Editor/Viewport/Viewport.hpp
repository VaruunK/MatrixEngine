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

	void GetEntityAtPixelLocation(float x, float y);

	float deltaSeconds = 0.0f;
private:
	const View& GetCameraView() const;

	Appstate& appstate;

	ViewportCamera camera;
	ViewportController controller;
	ViewportRenderer viewportRenderer;

	SDL_GPUTexture* offscreenTexture = nullptr;
	SDL_GPUSampler* offscreenSampler = nullptr;

	WorldRenderer* worldRenderer = nullptr;
};