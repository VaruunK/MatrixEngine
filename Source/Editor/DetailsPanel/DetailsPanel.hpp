#pragma once

#ifdef MATRIX_EDITOR
#include "Editor/DetailsPanel/DetailsPanelController/DetailsPanelController.hpp"
#include <array>

class Entity;
class Reflection;
struct Frame;
struct Field;

class DetailsPanel {
public:
	DetailsPanel();

	void SetEntityToView(Entity* entity);
	void Tick(float deltaTime);
	void Render(bool* active);
private:
	void RenderRightClickPopup();
	void RenderField(const Field& field);
	void RenderStructField(const Field& field, const Struct& reflection);
	void RenderClassField(const Field& field, const Class& reflection);

	DetailsPanelController controller;

	Entity* detailEntity = nullptr;

	std::array<std::string, 3> accessTypes = { "public", "protected", "private" };

	bool locked = false;
	bool unlocked = true;

	bool wasFocused = false;

	bool popup = false;
};

#endif