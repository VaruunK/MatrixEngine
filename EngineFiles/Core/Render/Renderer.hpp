#pragma once
#include "Core/ShaderManager/ShaderManager.hpp"
#include "PipelineEnums.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

class Texture;
class SpriteComponent;
class Window;

struct Vertex {
    glm::vec3 position;
    glm::vec2 uv;
};

class Renderer {
public:
    Renderer(SDL_GPUDevice* device);
    bool Initialize();
    SDL_GPUDevice* GetGPUDevice();
    void Render();
    void RegisterSprite(SpriteComponent* sprite);
    void DeregisterSprite(SpriteComponent* sprite);
    void Shutdown();

private:
    bool InitializeBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    bool InitializePipelines(Window* window, SDL_GPUShader* vertexShader, SDL_GPUShader* fragmentShader);
    bool InitializeSamplers();
    bool RenderTexture(const Texture* texture);

    SDL_GPUDevice* device = nullptr;
    SDL_GPUSampler* defaultSampler = nullptr;
    SDL_GPUBuffer* vertexBuffer = nullptr;
    SDL_GPUBuffer* indexBuffer = nullptr;
    uint32_t vertexBufferSize = 0;
    uint32_t indexBufferSize = 0;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<const Texture*, std::vector<SpriteComponent*>> spriteTextures;
    std::unordered_map<PIPELINE_TYPE, SDL_GPUGraphicsPipeline*> pipelines;
    static std::unique_ptr<ShaderManager> shaderManager;
};