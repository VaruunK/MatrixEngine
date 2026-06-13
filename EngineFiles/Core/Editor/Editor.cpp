#include "Editor.hpp"
#include "Core/Game/Game.hpp"
#include "Core/GameObject/World/World.hpp"

Editor::Editor(Appstate& appstate, Game* game) 
	: appstate(appstate), editorRenderer(appstate, game->world->GetWorldRenderer()) {
	
	this->game = game;
}

void Editor::Render() {
	editorRenderer.Render();
}

void Editor::Tick(float deltaTime) {
	editorRenderer.Tick(deltaTime);
}