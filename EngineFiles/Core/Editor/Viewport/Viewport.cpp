#include "Viewport.hpp"
#include "Core/Editor/Render/WorldRenderer/WorldRenderer.hpp"
#include "Core/Structs/Appstate.hpp"
#include "Core/Structs/View.hpp"
#include "Core/Structs/FrameData.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"
#include <iostream>

Viewport::Viewport(Appstate& appstate, WorldRenderer& worldRenderer) 
    : appstate(appstate), 
	worldRenderer(worldRenderer), 
	viewportRenderer(appstate, this, worldRenderer), 
	controller(this) {

    controller.Start();
}

void Viewport::Render(FrameData& frame) {
    
    frame.view = &GetCameraView();

    worldRenderer.Render(frame);
    viewportRenderer.Render(frame);
}

Entity* Viewport::GetSelectedEntity(int x, int y) {
    SDL_GPUCommandBuffer* pickCmd = SDL_AcquireGPUCommandBuffer(appstate.device);
    if (pickCmd) {
        viewportRenderer.RenderSelectProxy(pickCmd);
        Entity* picked = viewportRenderer.ReadPixel(pickCmd, x, y);
        /*if (picked) {
            std::cout << picked << std::endl;
        }*/
        return picked;
    }
    return nullptr;
}

void Viewport::Tick(float deltaTime) {
	this->deltaSeconds = deltaTime;
    controller.Tick(deltaTime);
}

void Viewport::SetCameraSpeed(int& speed) {
	camera.SetCameraSpeed(speed);
}

void Viewport::GetClickedPosition(int& x, int& y) {
    x = mouseClickX;
    y = mouseClickY;
}

void Viewport::SetClickedPosition(int x, int y) {
    mouseClickX = x;
    mouseClickY = y;
}

const View& Viewport::GetCameraView() const {
	return camera.GetCameraView();
}