#include "ViewportController.hpp"
#include "Core/ViewportCamera/ViewportCamera.hpp"
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_mouse.h>
#include <iostream>

ViewportController::ViewportController() : Controller() {
	
}

void ViewportController::Start(ViewportCamera* camera) {
	Controller::Start();
	viewportCamera = camera;
	BindMouseButton(SDL_BUTTON_RIGHT,
		[this]() { moveMode = true; }, 
		[this]() { moveMode = false; }
	);
	BindKey(SDL_SCANCODE_W,
		[this]() {
			if(moveMode) viewportCamera->MoveForward();
		});
	BindKey(SDL_SCANCODE_S,
		[this]() {
			if (moveMode) viewportCamera->MoveBackward();
		});
	BindKey(SDL_SCANCODE_A,
		[this]() {
			if (moveMode) viewportCamera->MoveLeft();
		});
	BindKey(SDL_SCANCODE_D,
		[this]() {
			if (moveMode) viewportCamera->MoveRight();
		});
	BindKey(SDL_SCANCODE_Q,
		[this]() {
			if (moveMode) viewportCamera->MoveUp();
		});
	BindKey(SDL_SCANCODE_E,
		[this]() {
			if (moveMode) viewportCamera->MoveDown();
		});
}

void ViewportController::Tick(uint64_t deltaTime) {
	Controller::Tick(deltaTime);
}

void ViewportController::DestroyGameObject() {
	Controller::DestroyGameObject();
}