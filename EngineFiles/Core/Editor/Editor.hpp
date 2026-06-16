#pragma once

#include "Core/Editor/Render/EditorRenderer/EditorRenderer.hpp"
#include "Core/Editor/Viewport/Viewport.hpp"

class Entity;
class Game;

struct Appstate;

class Editor {
public:
	Editor(Appstate& appstate, Game* game);
	~Editor() = default;

	void Tick(float deltaTime);
	void Render();

	EditorRenderer editorRenderer;
private:
	std::vector<Entity*> selectedEntities;
	
	Viewport viewport;

	Appstate& appstate;
	Game* game;
};