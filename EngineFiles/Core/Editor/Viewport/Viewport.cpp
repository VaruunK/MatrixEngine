#include "Viewport.hpp"
#include "Core/Editor/Render/WorldRenderer/WorldRenderer.hpp"
#include "Core/Structs/Appstate.hpp"
#include "Core/Structs/View.hpp"
#include "Core/Structs/FrameData.hpp"
#include <iostream>

Viewport::Viewport(Appstate& appstate, WorldRenderer& worldRenderer) 
    : appstate(appstate), 
	worldRenderer(worldRenderer), 
	viewportRenderer(appstate, this, worldRenderer), 
	controller(this) {

}

void Viewport::Initialize() {
    viewportRenderer.Initialize();
    controller.Start();

    ShaderManager* rawSM = new ShaderManager(appstate.device);
    if (!rawSM) {
        SDL_Log("Failed to create Shader Manager");
        return;
    }
}

void Viewport::Render(FrameData& frame) {
    
    frame.view = &GetCameraView();

    if (IsClicked()) {
        int x, y;
        HandleClick(x, y);

        SDL_GPUCommandBuffer* pickCmd = SDL_AcquireGPUCommandBuffer(appstate.device);
        if (pickCmd) {
            viewportRenderer.RenderSelectProxy(pickCmd);
            Entity* picked = viewportRenderer.ReadPixel(pickCmd, x, y);
            if (picked) {
                std::cout << picked << std::endl;
            }
        }
    }

    worldRenderer.Render(frame);
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

void Viewport::SetMouseClicked(int x, int y) {
	mouseClickX = x;
	mouseClickY = y;

	clicked = true;
}

void Viewport::HandleClick(int& mouseX, int& mouseY) {
	mouseX = mouseClickX;
	mouseY = mouseClickY;

	clicked = false;
}

const View& Viewport::GetCameraView() const {
	return camera.GetCameraView();
}