#include "Viewport.hpp"
#include "Engine.hpp"
#include "Core/Structs/View.hpp"
#include <iostream>

Viewport::Viewport(SDL_GPUDevice* device) : renderer(device, this){
	
}

void Viewport::Render(FrameData& frame) {
	renderer.Render(frame);

	if (renderer.IsActive()) {
		controller.Tick(frame.deltaTime);
	}
}

void Viewport::SetCameraSpeed(int& speed) {
	controller.GetCamera().SetCameraSpeed(speed);
}

void Viewport::Initialize() {
	renderer.Initialize();
	controller.Start();
}

const View& Viewport::GetCameraView() const {
	return controller.GetCamera().GetCameraView();
}
