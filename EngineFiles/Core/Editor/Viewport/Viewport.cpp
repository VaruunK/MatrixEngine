#include "Viewport.hpp"
#include "Core/Structs/Appstate.hpp"
#include "Core/Structs/View.hpp"
#include "Core/Structs/FrameData.hpp"
#include <SDL3/SDL_gpu.h>
#include <iostream>

Viewport::Viewport(Appstate& appstate, WorldRenderer& worldRenderer) 
    : appstate(appstate), 
	viewportRenderer(appstate, this, worldRenderer) {

	controller.BindMouseButton(SDL_BUTTON_RIGHT,
		[this]() { moveMode = true; },
		[this]() { moveMode = false; }
	);

	controller.BindKey(SDL_SCANCODE_W,
		[this]() {
			if (moveMode) this->GetCamera().MoveForward();
		});

	controller.BindKey(SDL_SCANCODE_S,
		[this]() {
			if (moveMode) this->GetCamera().MoveBackward();
		});

	controller.BindKey(SDL_SCANCODE_A,
		[this]() {
			if (moveMode) this->GetCamera().MoveLeft();
		});

	controller.BindKey(SDL_SCANCODE_D,
		[this]() {
			if (moveMode) this->GetCamera().MoveRight();
		});

	controller.BindKey(SDL_SCANCODE_Q,
		[this]() {
			if (moveMode) this->GetCamera().MoveUp();
		});

	controller.BindKey(SDL_SCANCODE_E,
		[this]() {
			if (moveMode) this->GetCamera().MoveDown();
		});

	controller.BindMouseMove([this](float relX, float relY) {
		if (moveMode) {
			this->GetCamera().RotateByMouse(relX, relY);
		}});

    controller.Start();
}

Viewport::~Viewport() {
    if (offscreenTexture) {
        SDL_ReleaseGPUTexture(appstate.device, offscreenTexture);
        offscreenTexture = nullptr;
    }
    if (offscreenSampler) {
        SDL_ReleaseGPUSampler(appstate.device, offscreenSampler);
        offscreenSampler = nullptr;
    }
}

void Viewport::Render(FrameData& frame) {
    frame.view = &GetCameraView();
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