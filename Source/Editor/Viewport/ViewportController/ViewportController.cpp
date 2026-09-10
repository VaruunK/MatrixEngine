#ifdef MATRIX_EDITOR

#include "ViewportController.hpp"
#include "Editor/Viewport/Viewport.hpp"
#include "Editor/Viewport/ViewportCamera/ViewportCamera.hpp"
#include <iostream>

ViewportController::ViewportController() : Controller() {

}

void ViewportController::Start() {
	Controller::Start();
}

void ViewportController::Tick(uint64_t deltaTime) {
	Controller::Tick(deltaTime);
}

void ViewportController::DestroyGameObject() {
	Controller::DestroyGameObject();
}

#endif