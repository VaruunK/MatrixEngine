#pragma once

#ifdef MATRIX_EDITOR
#include "Core/GameObject/Controller/Controller.hpp"

CLASS()
class DetailsPanelController : public Controller {
public:
	DetailsPanelController();

	FUNCTION()
	void Start() override;

	FUNCTION()
	void Tick(uint64_t deltaTime) override;

	FUNCTION()
	void DestroyGameObject() override;
private:

};

#endif