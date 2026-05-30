#include "ViewportController.hpp"
#include "Core/ViewportCamera/ViewportCamera.hpp"
#include <SDL3/SDL_scancode.h>
#include <iostream>

ViewportController::ViewportController() : Controller() {
	
}

void ViewportController::Start(ViewportCamera* camera) {
	Controller::Start();
	viewportCamera = camera;
	BindKey(SDL_SCANCODE_W, 
		[this]() {
			viewportCamera->MoveForward();
		});
	BindKey(SDL_SCANCODE_S,
		[this]() {
			viewportCamera->MoveBackward();
		});
	BindKey(SDL_SCANCODE_A,
		[this]() {
			viewportCamera->MoveLeft();
		});
	BindKey(SDL_SCANCODE_D,
		[this]() {
			viewportCamera->MoveRight();
		});
}

void ViewportController::Tick(uint64_t deltaTime) {
	Controller::Tick(deltaTime);
}

void ViewportController::DestroyGameObject() {
	Controller::DestroyGameObject();
}