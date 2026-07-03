#include "ViewportController.hpp"
#include "Core/Editor/Viewport/Viewport.hpp"
#include "Core/Editor/Viewport/ViewportCamera/Viewportcamera.hpp"
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