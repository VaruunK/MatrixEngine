#pragma once

#include "Core/Editor/Viewport/Viewport.hpp"

class WorldRenderer;
class Entity;

class EditorRenderer {
public:
	EditorRenderer(Appstate& appstate, WorldRenderer* worldRenderer);
	~EditorRenderer() = default;

	void Render();
	void Tick(float deltaTime);
private:
	ImGuiIO* io = nullptr;

	Appstate& appstate;

	Viewport viewport;

	SDL_GPUTexture* hitProxyTexture = nullptr;
	SDL_GPUGraphicsPipeline* hitProxyPipeline = nullptr;

	std::unordered_map<uint32_t, Entity*> idToEntity;

	uint32_t nextId = 1;

	uint32_t AssignId(Entity* entity);
	uint32_t ReadPixel(SDL_GPUCommandBuffer* cmd, int x, int y);
};