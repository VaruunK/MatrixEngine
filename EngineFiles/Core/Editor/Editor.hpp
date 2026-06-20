#pragma once

#include "Core/Editor/Render/EditorRenderer/EditorRenderer.hpp"
#include "Core/Editor/Viewport/Viewport.hpp"
#include "Core/Editor/ContentBrowser/ContentBrowser.hpp"
#include "Core/Structs/EditorInfo.hpp"
#include <set>

class Entity;
class Game;

struct Appstate;

class Editor {
public:
	Editor(Appstate& appstate, Game* game);
	~Editor();

	void Tick(float deltaTime);
	void Render();

	EditorRenderer editorRenderer;
private:

	EditorInfo info = {
		.viewport = viewport,
		.contentBrowser = contentBrowser
	};

	enum EditorItem {
		VIEWPORT,
		CONTENT_BROWSER,
		NONE
	};

	EditorItem focusedItem = NONE;

	std::set<Entity*> selectedEntities;
	
	Viewport viewport;
	ContentBrowser contentBrowser;

	Appstate& appstate;
	Game* game;
};