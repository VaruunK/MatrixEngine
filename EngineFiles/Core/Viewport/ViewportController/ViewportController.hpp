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

	const View& GetCameraView() const;
private:
	ViewportCamera camera;
	bool moveMode = false;
};