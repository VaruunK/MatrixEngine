#include "ContentBrowserController.hpp"
#include "Core/Editor/ContentBrowser/ContentBrowser.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"

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

