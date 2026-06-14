#include "Viewport.hpp"
#include "Core/Editor/Render/WorldRenderer/WorldRenderer.hpp"
#include "Core/Structs/Appstate.hpp"
#include "Core/Structs/View.hpp"
#include "Core/Structs/FrameData.hpp"
#include <iostream>

Viewport::Viewport(Appstate& appstate, WorldRenderer* worldRenderer) 
    : viewportRenderer(appstate, this), controller(this), appstate(appstate) {
    this->worldRenderer = worldRenderer;
}

void Viewport::Render(FrameData& frame) {
    
    frame.view = &GetCameraView();

    worldRenderer->Render(frame);
    viewportRenderer.Render(frame);
}

void Viewport::Tick(float deltaTime) {
	this->deltaSeconds = deltaTime;
	if (viewportRenderer.IsActive()) {
		controller.Tick(deltaTime);
	}
}

void Viewport::SetCameraSpeed(int& speed) {
	camera.SetCameraSpeed(speed);
}

void Viewport::Initialize() {
    viewportRenderer.Initialize();
	controller.Start();
}

const View& Viewport::GetCameraView() const {
	return camera.GetCameraView();
}
