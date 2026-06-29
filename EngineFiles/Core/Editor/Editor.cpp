#include "Editor.hpp"
#include "Core/Game/Game.hpp"
#include "Core/GameObject/World/World.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"
#include <iostream>

// does editor need selected objects? or viewport? 
// what owns gizmos? gizmo state?

Editor::Editor(Appstate& appstate, Game* game)
	: appstate(appstate),
	viewport(appstate, game->world.GetWorldRenderer()),
	editorRenderer(appstate, info, game->world.GetWorldRenderer()) {

	this->game = game;

	GEventBUS.Subscribe(EVENT_VIEWPORT_HOVERED, [this]() {
		focusedItem = VIEWPORT; }
	);

	GEventBUS.Subscribe(EVENT_CONTENT_BROWSER_HOVERED, [this]() {
		focusedItem = CONTENT_BROWSER; }
	);

	GEventBUS.Subscribe(EVENT_VIEWPORT_CLICKED, [this]() {
		int x, y;
		viewport.GetClickedPosition(x, y);
		Entity* entity = viewport.GetSelectedEntity(x, y);
		if (entity) {
			if (selectedEntities.contains(entity)) {
				selectedEntities.erase(entity);
			} else {
				selectedEntities.insert(entity);
			}
			for (Entity* e : selectedEntities) {
				std::cout << e << std::endl;
			}
		} else {
			selectedEntities.clear();
		}
	});
}

Editor::~Editor() {
	selectedEntities.clear();
}

void Editor::Render() {
	editorRenderer.Render();
}

void Editor::Tick(float deltaTime) {
	switch(focusedItem) {
	
	case VIEWPORT:
		viewport.Tick(deltaTime);
		break;
	case CONTENT_BROWSER:
		contentBrowser.Tick(deltaTime);
		break;
	default:
		break;
	}
}