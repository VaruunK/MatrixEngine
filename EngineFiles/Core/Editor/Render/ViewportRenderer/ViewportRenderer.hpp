#pragma once
#include "Core/ShaderManager/ShaderManager.hpp"
#include "Core/Enums/PipelineEnums.hpp"
#include "Core/Structs/RenderStructs.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

class Viewport;
class WorldRenderer;
class Entity;
class MeshComponent;
class SpriteComponent;

struct ImGuiIO;
struct FrameData;
struct Appstate;

class ViewportRenderer {
public:
	ViewportRenderer(Appstate& appstate, Viewport* viewport, WorldRenderer& worldRenderer);
	~ViewportRenderer();
	bool Initialize();

	void Render(FrameData& frame);
	void RenderSelectProxy(SDL_GPUCommandBuffer* commandBuffer);
	
	Entity* ReadPixel(SDL_GPUCommandBuffer* commandBuffer, int x, int y);

	void Shutdown();
	bool IsActive() { return active; }
	bool resized = false;
	
private:
	bool InitializeBuffers();
	bool InitializeSamplers();
	bool InitializeSelectProxyPipeline(SDL_GPUShader* vertexShader, SDL_GPUShader* fragmentShader);
	
	bool CreateSelectProxyTexture();
	bool CreateSelectProxyDepthTexture();

	SDL_GPUTextureFormat GetDepthStencilFormat();

	Appstate& appstate;
	ImGuiIO* io = nullptr;
	Viewport* viewport;
	WorldRenderer& worldRenderer;
	static std::unique_ptr<ShaderManager> shaderManager;

	SDL_GPUSampler* defaultSampler = nullptr;

	SDL_GPUTexture* selectProxyTexture = nullptr;
	SDL_GPUTexture* selectProxyDepthTexture = nullptr;
	SDL_GPUGraphicsPipeline* selectProxyPipeline = nullptr;

	std::unordered_map<uint32_t, Entity*> idToEntity;

	// ID mapping
	std::unordered_map<uint32_t, MeshComponent*>   idToMesh;
	std::unordered_map<uint32_t, SpriteComponent*> idToSprite;
	uint32_t nextId = 1;

	void ResetIds() { idToMesh.clear(); idToSprite.clear(); nextId = 1; }

	glm::vec4 EncodeId(uint32_t id) {
		return {
			((id >> 0) & 0xFF) / 255.f,
			((id >> 8) & 0xFF) / 255.f,
			((id >> 16) & 0xFF) / 255.f,
			1.0f
		};
	}

	uint32_t DecodeId(uint8_t r, uint8_t g, uint8_t b) {
		return (uint32_t)r | ((uint32_t)g << 8) | ((uint32_t)b << 16);
	}

	bool msaaEnabled = true;

	bool show_demo_window = true;

	bool active = true;
	bool showFPS = false;
	int speed = 1;
};