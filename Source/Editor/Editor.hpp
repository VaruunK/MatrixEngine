#pragma once

#ifdef MATRIX_EDITOR

#include "Editor/EditorRenderer/EditorRenderer.hpp"
#include "Editor/Viewport/Viewport.hpp"
#include "Editor/ContentBrowser/ContentBrowser.hpp"
#include "Editor/DetailsPanel/DetailsPanel.hpp"
#include "Editor/OutlinerPanel/OutlinerPanel.hpp"
#include "Structs/EditorInfo.hpp"
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

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

	void FirstEntityName(Entity* entity);
	// std::string GetEntityName(Entity* entity);

	EditorInfo info = {
		.viewport = viewport,
		.contentBrowser = contentBrowser,
		.detailsPanel = detailsPanel,
		.outlinerPanel = outlinerPanel
	};

	enum EditorItem {
		VIEWPORT,
		CONTENT_BROWSER,
		DETAILS_PANEL,
		OUTLINER_PANEL,
		NONE
	};

	EditorItem focusedItem = NONE;

	std::set<Entity*> selectedEntities;
	std::unordered_map<std::string, std::unordered_map<Entity*, std::string>> entityNames;

	std::unordered_map<std::filesystem::path, Mesh*> meshes;
	std::unordered_map<std::filesystem::path, Texture*> textures;
	std::unordered_map<std::filesystem::path, Material*> materials;

	EditorRenderer editorRenderer;
	Viewport viewport;
	ContentBrowser contentBrowser;
	DetailsPanel detailsPanel;
	OutlinerPanel outlinerPanel;

	Appstate& appstate;
	Game* game;
};

#endif