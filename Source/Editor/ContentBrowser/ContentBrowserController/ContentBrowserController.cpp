#include "ContentBrowserController.hpp"

#ifdef MATRIX_EDITOR

ContentBrowserController::ContentBrowserController() : Controller() {
}

void ContentBrowserController::Start() {
	Controller::Start();
}

void ContentBrowserController::Tick(uint64_t deltaTime) {
	Controller::Tick(deltaTime);
}

void ContentBrowserController::DestroyGameObject() {
	Controller::DestroyGameObject();
}

#endif