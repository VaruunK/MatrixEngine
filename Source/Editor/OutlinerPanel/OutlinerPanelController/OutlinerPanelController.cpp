#include "OutlinerPanelController.hpp"

#ifdef MATRIX_EDITOR

OutlinerPanelController::OutlinerPanelController() : Controller() {
}

void OutlinerPanelController::Start() {
	Controller::Start();
}

void OutlinerPanelController::Tick(uint64_t deltaTime) {
	Controller::Tick(deltaTime);
}

void OutlinerPanelController::DestroyGameObject() {
	Controller::DestroyGameObject();
}

#endif