#pragma once

#ifdef MATRIX_EDITOR
#include "Core/Editor/DetailsPanel/DetailsPanelController/DetailsPanelController.hpp"

class Entity;
struct Frame;

class DetailsPanel {
public:
	DetailsPanel();

	void SetEntityToView(Entity* entity);
	void Tick(float deltaTime);
	void Render(bool* active);
private:
	void RenderRightClickPopup();

	DetailsPanelController controller;

	Entity* detailEntity = nullptr;

	bool locked = false;
	bool unlocked = true;

	bool wasFocused = false;

	bool popup = false;
};

#endif