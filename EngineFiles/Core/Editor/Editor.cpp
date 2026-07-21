#include "Editor.hpp"
#include "Core/Game/Game.hpp"
#include "Core/GameObject/World/World.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"
#include <iostream>
#include <array>
#include <string>

// does editor need selected objects? or viewport? 
// what owns gizmos? gizmo state?

Editor::Editor(Appstate& appstate, Game* game)
	: appstate(appstate), contentBrowser(appstate),
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
			std::array ar = { "public", "protected", "private" };
			for (auto* entity : selectedEntities) {
				const ReflectedClass& rc = entity->GetClass();

				std::cout << "Name: " << rc.name << std::endl;
				std::cout << "Parent: " << rc.parent << std::endl;

				for (auto& access : ar) {
					std::cout << access << " fields" << std::endl;
					auto it = rc.fields.find(access);
					if (it != rc.fields.end()) {
						for (auto& field : it->second) {
							std::cout << field.typeName << ": " << field.name << std::endl;
						}
					}
					std::cout << "\n" << std::endl;
				}

				for (auto& access : ar) {
					std::cout << access << " functions" << std::endl;
					auto it = rc.functions.find(access);
					if (it != rc.functions.end()) {
						for (auto& func : it->second) {
							std::cout << func.returnType << ": " << func.name << std::endl;
							if (func.arguments.size() > 0) {
								for (auto& [name, argType] : func.arguments) {
									std::cout << "\t" << argType << ": " << name << std::endl;
								}
							}
						}
					}
					std::cout << "\n" << std::endl;
				}
			}
			
		} else {
			selectedEntities.clear();
		}
	});

	GEventBUS.Subscribe(EVENT_GAME_START, [this]() {
		this->game->Start();
		});

	GEventBUS.Subscribe(EVENT_GAME_END, [this]() {
		this->game->Quit();
		});
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