#pragma once

#include "Core/GameObject/Controller/Controller.hpp"
#include "Core/Viewport/ViewportCamera/ViewportCamera.hpp"

class ViewportController : public Controller {
public:
	ViewportController();
	~ViewportController() = default;

	void Start();
	void Tick(uint64_t deltaTime) override;
	
	void DestroyGameObject() override;

	ViewportCamera& GetCamera() { return camera; }
	const ViewportCamera& GetCamera() const { return camera; }
private:
	ViewportCamera camera;
	bool leftClicking = false;
	bool moveMode = false;
};