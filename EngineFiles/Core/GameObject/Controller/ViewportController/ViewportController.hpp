#pragma once

#include "Core/GameObject/Controller/Controller.hpp"

class ViewportCamera;

class ViewportController : public Controller {
public:
	ViewportController();
	~ViewportController() = default;

	void Start(ViewportCamera* camera);
	void Tick(uint64_t deltaTime) override;
	void DestroyGameObject() override;
private:
	ViewportCamera* viewportCamera;
	bool moveMode = false;
};