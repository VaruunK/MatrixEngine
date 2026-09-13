#pragma once

#ifdef MATRIX_EDITOR

#include "Editor/DetailsPanel/DetailsPanelController/DetailsPanelController.hpp"
#include <string>
#include <unordered_map>
#include <array>
#include <set>

class GameObject;
class Reflection;
class Entity;

struct Frame;
struct Field;

class DetailsPanel {
public:
	DetailsPanel(std::unordered_map<std::string, std::unordered_map<Entity*, std::string>>* entityNames, std::set<Entity*>* selectedEntities);

	void Tick(float deltaTime);
	void Render(bool* active);
private:
	void SetGameObjectToView(GameObject* gameObject, const std::string& name);

	void RenderRightClickPopup();
	void RenderInheritedFields(const Class& rc, bool renderProtected);
	void RenderField(const Field& field);
	void RenderStructField(const Field& field, const Struct& reflection);
	void RenderClassField(const Field& field, const Class& reflection);
	
	const Field* FindFieldByName(const Class& rc, const std::string& name);
	std::string GetEntityName(Entity *entity);

	std::unordered_map<std::string, std::unordered_map<Entity*, std::string>>* entityNames;
	std::set<Entity*>* selectedEntities;

	DetailsPanelController controller;

	GameObject* detailGameObject = nullptr;
	std::string gameObjectName;

	std::array<std::string, 3> accessTypes = { "public", "protected", "private" };

	bool locked = false;
	bool unlocked = true;

	bool wasFocused = false;

	bool popup = false;
};

#endif