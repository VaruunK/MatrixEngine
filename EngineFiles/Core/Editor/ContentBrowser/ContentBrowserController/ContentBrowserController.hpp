#pragma once

#ifdef MATRIX_EDITOR
#include "Core/GameObject/Controller/Controller.hpp"

class ContentBrowserController : public Controller {
public:
	ContentBrowserController();

	void Start() override;
	void Tick(uint64_t deltaTime) override;

	void DestroyGameObject() override;
private:
};

#endif