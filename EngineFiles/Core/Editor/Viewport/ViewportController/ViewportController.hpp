#pragma once

#include "Core/GameObject/Controller/Controller.hpp"

class Viewport;

class ViewportController : public Controller {
public:
	ViewportController();
	~ViewportController() = default;

	void Start() override;
	void Tick(uint64_t deltaTime) override;
	
	void DestroyGameObject() override;
private:
};