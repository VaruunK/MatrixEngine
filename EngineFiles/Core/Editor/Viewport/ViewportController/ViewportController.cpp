#include "ViewportController.hpp"
#include "Core/Editor/Viewport/Viewport.hpp"
#include "Core/Editor/Viewport/ViewportCamera/Viewportcamera.hpp"
#include <iostream>

ViewportController::ViewportController(Viewport* viewport) : Controller() {
	this->viewport = viewport;
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
			};
		},
		[this]() {
			if (!moveMode && leftClicking) {
				leftClicking = false;
			}
		});
	BindKey(SDL_SCANCODE_W,
		[this]() {
			if(moveMode) viewport->GetCamera().MoveForward();
		});
	BindKey(SDL_SCANCODE_S,
		[this]() {
			if (moveMode) viewport->GetCamera().MoveBackward();
		});
	BindKey(SDL_SCANCODE_A,
		[this]() {
			if (moveMode) viewport->GetCamera().MoveLeft();
		});
	BindKey(SDL_SCANCODE_D,
		[this]() {
			if (moveMode) viewport->GetCamera().MoveRight();
		});
	BindKey(SDL_SCANCODE_Q,
		[this]() {
			if (moveMode) viewport->GetCamera().MoveUp();
		});
	BindKey(SDL_SCANCODE_E,
		[this]() {
			if (moveMode) viewport->GetCamera().MoveDown();
		});
	BindMouseMove([this](float relX, float relY) {
		if (moveMode) {
			viewport->GetCamera().RotateByMouse(relX, relY);
		}});
}

void ViewportController::Tick(uint64_t deltaTime) {
	Controller::Tick(deltaTime);
}

void ViewportController::DestroyGameObject() {
	Controller::DestroyGameObject();
}