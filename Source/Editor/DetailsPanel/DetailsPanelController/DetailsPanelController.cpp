#include "DetailsPanelController.hpp"

#ifdef MATRIX_EDITOR

DetailsPanelController::DetailsPanelController() : Controller() {
}

void DetailsPanelController::Start() {
	Controller::Start();
}

void DetailsPanelController::Tick(uint64_t deltaTime) {
	Controller::Tick(deltaTime);
}

void DetailsPanelController::DestroyGameObject() {
	Controller::DestroyGameObject();
}

#endif