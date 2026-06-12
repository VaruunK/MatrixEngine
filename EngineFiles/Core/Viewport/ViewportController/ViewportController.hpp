#pragma once

#include "Core/GameObject/Controller/Controller.hpp"
#include "Core/Viewport/ViewportCamera/ViewportCamera.hpp"

class Viewport;
class ViewportCamera;

class ViewportController : public Controller {
public:
	ViewportController(Viewport* viewport, ViewportCamera* camera);
	~ViewportController() = default;

	void Start();
	void Tick(uint64_t deltaTime) override;
	
	void DestroyGameObject() override;
private:
	Viewport* viewport;
	ViewportCamera* camera;

	bool leftClicking = false;
	bool moveMode = false;
};