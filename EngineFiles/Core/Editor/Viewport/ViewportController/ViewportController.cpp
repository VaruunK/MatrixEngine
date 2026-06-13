#include "ViewportController.hpp"
#include "Core/Editor/Viewport/Viewport.hpp"
#include "Core/Editor/Viewport/ViewportCamera/Viewportcamera.hpp"
#include <iostream>

ViewportController::ViewportController(Viewport* viewport, ViewportCamera* camera) : Controller() {
	this->viewport = viewport;
	this->camera = camera;
}

void ViewportController::Start() {
	Controller::Start();
	BindMouseButton(SDL_BUTTON_RIGHT,
		[this]() { moveMode = true; }, 
		[this]() { moveMode = false; }
	);
	BindMouseButton(SDL_BUTTON_LEFT,
		[this]() {
			if (!moveMode && !leftClicking) {
				leftClicking = true;
				viewport->GetEntityAtPixelLocation(GetMouseX(), GetMouseY());
			};
		},
		[this]() {
			if (!moveMode && leftClicking) leftClicking = false;
		});
	BindKey(SDL_SCANCODE_W,
		[this]() {
			if(moveMode) camera->MoveForward();
		});
	BindKey(SDL_SCANCODE_S,
		[this]() {
			if (moveMode) camera->MoveBackward();
		});
	BindKey(SDL_SCANCODE_A,
		[this]() {
			if (moveMode) camera->MoveLeft();
		});
	BindKey(SDL_SCANCODE_D,
		[this]() {
			if (moveMode) camera->MoveRight();
		});
	BindKey(SDL_SCANCODE_Q,
		[this]() {
			if (moveMode) camera->MoveUp();
		});
	BindKey(SDL_SCANCODE_E,
		[this]() {
			if (moveMode) camera->MoveDown();
		});
	BindMouseMove([this](float relX, float relY) {
		if (moveMode) {
			camera->RotateByMouse(relX, relY);
		}});
}

void ViewportController::Tick(uint64_t deltaTime) {
	Controller::Tick(deltaTime);
}

void ViewportController::DestroyGameObject() {
	Controller::DestroyGameObject();
}