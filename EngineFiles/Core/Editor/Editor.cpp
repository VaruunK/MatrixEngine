#include "Editor.hpp"
#include "Core/Game/Game.hpp"
#include "Core/GameObject/World/World.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"
#include <iostream>

// What owns the controller? 
// from there, how do items communicate? BUS? input router? 
// how should state be managed? enum? bool? where does state come from? how is it passed along the chain?
// does editor need selected objects? or viewport? 
// what owns gizmos? gizmo state?

Editor::Editor(Appstate& appstate, Game* game)
	: appstate(appstate),
	viewport(appstate, game->world.GetWorldRenderer()),
	editorRenderer(appstate, viewport, game->world.GetWorldRenderer()) {

	this->game = game;

	viewport.Initialize();

	GEventBUS.Subscribe(SDL_EVENT_USER, [this]() { 
		int x, y;
		viewport.GetClickedPosition(x, y);
		Entity* entity = viewport.GetSelectedEntity(x, y);
		if (entity) {
			selectedEntities.insert(entity);
			for (Entity* e : selectedEntities) {
				std::cout << e << std::endl;
			}
		} else {
			selectedEntities.clear();
		}
	});
}

void Editor::Render() {
	editorRenderer.Render();
}

void Editor::Tick(float deltaTime) {
	viewport.Tick(deltaTime);
}