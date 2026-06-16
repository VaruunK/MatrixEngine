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
	Viewport(Appstate& appstate, WorldRenderer& worldRenderer);
	~Viewport() = default;

	void Initialize();
	void Tick(float deltaTime);
	void Render(FrameData& frame);

	bool InitializeSelectProxyPipeline(SDL_GPUShader* vertexShader, SDL_GPUShader* fragmentShader);
	bool CreateSelectProxyTexture();
	bool CreateSelectProxyDepthTexture();

	SDL_GPUTextureFormat GetDepthStencilFormat();

	void SetCameraSpeed(int& speed);

	bool IsClicked() const { return clicked; }
	void SetMouseClicked(int x, int y);
	void HandleClick(int& mouseX, int& mouseY);

	const View& GetCameraView() const;
	ViewportCamera& GetCamera() { return camera; }

	float deltaSeconds = 0.0f;

	bool resized = false;
private:

	Appstate& appstate;

	ViewportCamera camera;
	ViewportController controller;
	ViewportRenderer viewportRenderer;

	SDL_GPUTexture* offscreenTexture = nullptr;
	SDL_GPUSampler* offscreenSampler = nullptr;

	WorldRenderer& worldRenderer;

	int mouseClickX = -1;
	int mouseClickY = -1;

	bool clicked = false;
};