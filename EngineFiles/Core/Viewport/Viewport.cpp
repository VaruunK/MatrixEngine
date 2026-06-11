#include "Viewport.hpp"
#include "Core/GameObject/World/WorldRenderer/WorldRenderer.hpp"
#include "Core/Structs/View.hpp"
#include <iostream>

Viewport::Viewport(SDL_GPUDevice* device, SDL_Window* window) : renderer(device, window, this){
    this->device = device;
    this->window = window;
}

void Viewport::Render() {
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
    if (!cmd) return;

    SDL_GPUTexture* swapchain = nullptr;
    SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &swapchain, nullptr, nullptr);

    FrameData frame{};
    frame.commandBuffer = cmd;
    frame.swapchainTexture = swapchain;
    frame.view = &GetCameraView();

    if (swapchain) {
        worldRenderer->Render(frame);
        renderer.Render(frame);
    }

    if (!SDL_SubmitGPUCommandBuffer(cmd)) {
        SDL_Log("Failed to submit: %s", SDL_GetError());
    }
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
