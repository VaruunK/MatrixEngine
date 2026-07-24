#pragma once

#ifdef MATRIX_EDITOR

#include "Core/Editor/EditorRenderer/EditorRenderer.hpp"
#include "Core/Editor/Viewport/Viewport.hpp"
#include "Core/Editor/ContentBrowser/ContentBrowser.hpp"
#include "Core/Editor/DetailsPanel/DetailsPanel.hpp"
#include "Core/Structs/EditorInfo.hpp"
#include <set>

class Entity;
class Game;

struct Appstate;
struct Texture;
struct Material;
struct Mesh;

class Editor {
public:
	Editor(Appstate& appstate, Game* game);
	~Editor();

	void Tick(float deltaTime);
	void Render();
private:

	EditorInfo info = {
		.viewport = viewport,
		.contentBrowser = contentBrowser,
		.detailsPanel = detailsPanel
	};

	enum EditorItem {
		VIEWPORT,
		CONTENT_BROWSER,
		DETAILS_PANEL,
		NONE
	};

	EditorItem focusedItem = NONE;

	std::set<Entity*> selectedEntities;
	
	std::unordered_map<std::filesystem::path, Mesh*> meshes;
	std::unordered_map<std::filesystem::path, Texture*> textures;
	std::unordered_map<std::filesystem::path, Material*> materials;

	EditorRenderer editorRenderer;
	Viewport viewport;
	ContentBrowser contentBrowser;
	DetailsPanel detailsPanel;

	Appstate& appstate;
	Game* game;
};

#endif