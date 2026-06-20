#pragma once
#include "Core/GameObject/Controller/Controller.hpp"

class ContentBrowser;

class ContentBrowserController : public Controller {
	friend class ContentBrowser;
public:
	ContentBrowserController(ContentBrowser* contentBrowser);

	void Start() override;
	void Tick(uint64_t deltaTime) override;

	void DestroyGameObject() override;
private:
	ContentBrowser* contentBrowser;
};