#pragma once

#ifdef MATRIX_EDITOR
#include "Core/GameObject/Controller/Controller.hpp"

class Viewport;

CLASS()
class ViewportController : public Controller {
public:
	ViewportController();
	~ViewportController() = default;

	FUNCTION()
	void Start() override;

	FUNCTION()
	void Tick(uint64_t deltaTime) override;
	
	FUNCTION()
	void DestroyGameObject() override;
private:
};

#endif