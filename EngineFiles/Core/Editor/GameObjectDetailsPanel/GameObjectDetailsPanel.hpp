#pragma once

#ifdef MATRIX_EDITOR
#include "Core/Editor/GameObjectDetailsPanel/GameObjectDetailsPanelController/GameObjectDetailsPanelController.hpp"

class GameObject;
struct Frame;

class GameObjectDetailsPanel {
public:
	GameObjectDetailsPanel();

	void SetGameObjectToView(GameObject* gameObject);
	void Tick(float deltaTime);
	void Render(bool* active);
private:
	void RenderRightClickPopup();

	GameObjectDetailsPanelController controller;

	GameObject* detailGameObject = nullptr;

	bool locked = false;
	bool unlocked = true;

	bool wasFocused = false;

	bool popup = false;
};

#endif