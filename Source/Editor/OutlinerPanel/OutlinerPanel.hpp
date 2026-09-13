#pragma once

#ifdef MATRIX_EDITOR

#include "OutlinerPanelController/OutlinerPanelController.hpp"
#include <unordered_map>
#include <string>
#include <set>

class GameObject;
class Reflection;
class Entity;
class Game;

struct Appstate;

class OutlinerPanel {
public:
    OutlinerPanel(std::unordered_map<std::string, std::unordered_map<Entity*, std::string>>* entityNames, std::set<Entity*>* selectedEntities);
    void Render(bool* active);
    void Tick(float deltaTime);
private:
    void RenderRightClickPopup();
    const Field* FindFieldByName(const Class& rc, const std::string& name);

    std::unordered_map<std::string, std::unordered_map<Entity*, std::string>>* entityNames;
    std::set<Entity*>* selectedEntities;

    OutlinerPanelController controller;

    std::array<std::string, 3> accessTypes = { "public", "protected", "private" };

    bool locked = false;
	bool unlocked = true;

	bool wasFocused = false;

	bool popup = false;
};

#endif