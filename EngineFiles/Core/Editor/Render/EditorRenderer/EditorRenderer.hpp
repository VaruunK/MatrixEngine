#pragma once
#include "Core/Editor/Viewport/Viewport.hpp"
#include "Core/ShaderManager/ShaderManager.hpp"

class WorldRenderer;
class Entity;
class MeshComponent;
class SpriteComponent;

class EditorRenderer {
public:
	EditorRenderer(Appstate& appstate, WorldRenderer* worldRenderer);
	~EditorRenderer();

	void Render();
    void Shutdown();
	void Tick(float deltaTime);

    bool resized = false;
private:
	bool InitializeSelectProxyPipeline(SDL_GPUShader* vertexShader, SDL_GPUShader* fragmentShader);
	bool CreateSelectProxyTexture();
    bool CreateSelectProxyDepthTexture();

	SDL_GPUTextureFormat GetDepthStencilFormat();

	ImGuiIO* io = nullptr;

	Appstate& appstate;

	Viewport viewport;

	SDL_GPUTexture* selectProxyTexture = nullptr;
    SDL_GPUTexture* selectProxyDepthTexture = nullptr;
	SDL_GPUGraphicsPipeline* selectProxyPipeline = nullptr;

	WorldRenderer* worldRenderer = nullptr;

	std::unordered_map<uint32_t, Entity*> idToEntity;

	static std::unique_ptr<ShaderManager> shaderManager;

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

    void RenderSelectProxy(SDL_GPUCommandBuffer* commandBuffer);
    Entity* ReadPixel(SDL_GPUCommandBuffer* commandBuffer, int x, int y);
};