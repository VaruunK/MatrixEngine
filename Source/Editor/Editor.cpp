#include "Editor.hpp"
#include "Core/Game/Game.hpp"
#include "Core/GameObject/World/World.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"
#include "Core/Statics/GameStatics.hpp"
#include <iostream>
#include <array>
#include <string>

#ifdef MATRIX_EDITOR

// does editor need selected objects? or viewport? 
// what owns gizmos? gizmo state?

Editor::Editor(Appstate& appstate, Game* game) : 
	appstate(appstate), 
	contentBrowser(appstate),
	viewport(appstate, game->world.GetWorldRenderer()),
	detailsPanel(&entityNames, &selectedEntities),
	outlinerPanel(&entityNames, &selectedEntities),
	editorRenderer(appstate, info, game->world.GetWorldRenderer()) {

	this->game = game;

	GEventBUS.Subscribe(EVENT_VIEWPORT_HOVERED, [this]() {
		focusedItem = VIEWPORT; }
	);

	GEventBUS.Subscribe(EVENT_CONTENT_BROWSER_HOVERED, [this]() {
		focusedItem = CONTENT_BROWSER; }
	);

	GEventBUS.Subscribe(EVENT_DETAILS_PANEL_HOVERED, [this]() {
		focusedItem = DETAILS_PANEL; }
	);

	GEventBUS.Subscribe(EVENT_OUTLINER_PANEL_HOVERED, [this]() {
		focusedItem = OUTLINER_PANEL; }
	);

	// idea for handling multiple selection possibilities. 
	// rather than exposing editor or details panels to outliner panel and to each other creating tight coupling
	// pass in selected entities
	// this can be done via pointer passed in during construction or through tick(probably not good idea)
	// details panels then can handle setting gameobject to view by itself without getting rid of need for events
	// outliner can then update pointer where differences can then be displayed by details panel in next tick

	GEventBUS.Subscribe(EVENT_VIEWPORT_CLICKED, [this]() {
		int x, y;
		viewport.GetClickedPosition(x, y);
		Entity* entity = viewport.GetSelectedEntity(x, y);
		if (entity) {
			if(selectedEntities.empty()) {
                selectedEntities.insert(entity);
            } else {
            	selectedEntities.clear();
                selectedEntities.insert(entity);
                }
		} else {
			selectedEntities.clear();
		}
	});

	GEventBUS.Subscribe(EVENT_GAME_START, [this]() {
		this->game->Start(); }
	);

	GEventBUS.Subscribe(EVENT_GAME_END, [this]() {
		this->game->Quit(); }
	);

	for(const auto& [entityClass, entityList] : currentLevel->GetAllEntities()) {
		for(Entity* entity : entityList) {
			FirstEntityName(entity);
		}
	}
}

Editor::~Editor() {
	for (auto& [path, texture] : textures) {
		if (texture) {
			if (texture->texture) {
				SDL_ReleaseGPUTexture(appstate.device, texture->texture);
				texture->texture = nullptr;
			}
			if (texture->data) {
				SDL_DestroySurface(texture->data);
				texture->data = nullptr;
			}
			delete texture;
			texture = nullptr;
		}
	}
	for (auto& [path, mesh] : meshes) {
		if (mesh) {
			mesh->vertices.clear();
			mesh->indices.clear();
			delete mesh;
			mesh = nullptr;
		}
	}

	for (auto& [path, material] : materials) {
		if (material) {
			delete material;
			material = nullptr;
		}
	}
	textures.clear();
	meshes.clear();
	materials.clear();

	selectedEntities.clear();
}

void Editor::Render() {
	editorRenderer.Render();
}

void Editor::FirstEntityName(Entity* entity) {
	std::string typeName = entity->GetClass().typeInfo.get()->typeName;
    std::string entityName;

	entityName = typeName + "(" + std::to_string(entityNames[typeName].size()) + ")";

	entityNames[typeName][entity] = entityName;
}

// std::string Editor::GetEntityName(Entity *entity) {
// 	std::string typeName = entity->GetClass().typeInfo.get()->typeName;
    
// 	if(entityNames.contains(typeName)) {
// 		if(entityNames[typeName].contains(entity)) {
// 			return entityNames[typeName][entity];
// 		}
// 	}
// 	return "";
// }

void Editor::Tick(float deltaTime) {
	switch(focusedItem) {
	
	case VIEWPORT:
		viewport.Tick(deltaTime);
		break;
	case CONTENT_BROWSER:
		contentBrowser.Tick(deltaTime);
		break;
	case DETAILS_PANEL:
		detailsPanel.Tick(deltaTime);
		break;
	case OUTLINER_PANEL:
		outlinerPanel.Tick(deltaTime);
		break;
	default:
		break;
	}
}

#endif