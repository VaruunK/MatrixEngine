#pragma once

#ifdef MATRIX_EDITOR

#include "Editor/DetailsPanel/DetailsPanelController/DetailsPanelController.hpp"
#include <array>

class GameObject;
class Reflection;
struct Frame;
struct Field;

class DetailsPanel {
public:
	DetailsPanel();

	void SetGameObjectToView(GameObject* gameObject);
	void Tick(float deltaTime);
	void Render(bool* active);
private:
	void RenderRightClickPopup();

	void RenderInheritedFields(const Class& rc, bool renderProtected);
	void RenderField(const Field& field);
	void RenderStructField(const Field& field, const Struct& reflection);
	void RenderClassField(const Field& field, const Class& reflection);
	const Field* FindFieldByName(const Class& rc, const std::string& name);

	DetailsPanelController controller;

	GameObject* detailGameObject = nullptr;

	std::array<std::string, 3> accessTypes = { "public", "protected", "private" };

	bool locked = false;
	bool unlocked = true;

	bool wasFocused = false;

	bool popup = false;
};

#endif