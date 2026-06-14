#pragma once

#include "Core/GameObject/Controller/Controller.hpp"

class Viewport;

class ViewportController : public Controller {
public:
	ViewportController(Viewport* viewport);
	~ViewportController() = default;

	void Start();
	void Tick(uint64_t deltaTime) override;
	
	void DestroyGameObject() override;
private:
	Viewport* viewport;

	bool leftClicking = false;
	bool moveMode = false;
};