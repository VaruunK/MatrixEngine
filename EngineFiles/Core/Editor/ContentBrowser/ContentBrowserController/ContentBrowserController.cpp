#include "ContentBrowserController.hpp"
#include "Core/Editor/ContentBrowser/ContentBrowser.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"

ContentBrowserController::ContentBrowserController(ContentBrowser* contentBrowser) : Controller() {
	this->contentBrowser = contentBrowser;
}

void ContentBrowserController::Start() {
	Controller::Start();
	BindMouseButton(SDL_BUTTON_RIGHT,
		[this]() { contentBrowser->OpenLockPopup(); });

	BindMouseButton(SDL_BUTTON_LEFT, 
		[this]() { contentBrowser->CloseLockPopup(); });
}

void ContentBrowserController::Tick(uint64_t deltaTime) {
	Controller::Tick(deltaTime);
}

void ContentBrowserController::DestroyGameObject() {
	Controller::DestroyGameObject();
}

