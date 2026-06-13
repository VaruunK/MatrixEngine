#pragma once

#include "Core/Editor/Render/EditorRenderer/EditorRenderer.hpp"

class Entity;
class Game;

struct Appstate;

class Editor {
public:
	Editor(Appstate& appstate, Game* game);
	~Editor() = default;

	void Tick(float deltaTime);
	void Render();
private:
	std::vector<Entity*> selectedEntities;
	
	EditorRenderer editorRenderer;

	Appstate& appstate;
	Game* game;
};