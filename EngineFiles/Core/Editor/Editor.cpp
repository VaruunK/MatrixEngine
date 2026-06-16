#include "Editor.hpp"
#include "Core/Game/Game.hpp"
#include "Core/GameObject/World/World.hpp"

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
}

void Editor::Render() {
	editorRenderer.Render();
}

void Editor::Tick(float deltaTime) {
	viewport.Tick(deltaTime);
}