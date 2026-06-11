#include "Viewport.hpp"
#include "Core/GameObject/World/WorldRenderer/WorldRenderer.hpp"
#include "Core/Structs/View.hpp"
#include <iostream>

Viewport::Viewport(SDL_GPUDevice* device, SDL_Window* window) : renderer(device, window, this){
	
}

void Viewport::Render() {
	worldRenderer->frame.view = &GetCameraView();
	FrameData frame = worldRenderer->Render();

	renderer.Render(frame);
}

void Viewport::Tick(float deltaTime) {
	this->deltaSeconds = deltaTime;
	if (renderer.IsActive()) {
		controller.Tick(deltaTime);
	}
}

void Viewport::SetCameraSpeed(int& speed) {
	controller.GetCamera().SetCameraSpeed(speed);
}

void Viewport::Initialize(WorldRenderer* worldRenderer) {
	this->worldRenderer = worldRenderer;
	renderer.Initialize();
	controller.Start();
}

const View& Viewport::GetCameraView() const {
	return controller.GetCamera().GetCameraView();
}
