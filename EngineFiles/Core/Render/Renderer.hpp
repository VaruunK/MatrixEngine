#pragma once
#include "Core/ShaderManager/ShaderManager.hpp"
#include "Core/Assets/AssetStructs.hpp"
#include "PipelineEnums.hpp"
#include "RenderStructs.hpp"

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

class SpriteComponent;
class MeshComponent;
class Window;

class Renderer {
public:
    Renderer(SDL_GPUDevice* device);
    bool Initialize();
    SDL_GPUDevice* GetGPUDevice();
    void Render(glm::mat4 &projectionMatrix, glm::mat4 &viewMatrix);

    void RegisterSprite(SpriteComponent* sprite);
    void DeregisterSprite(SpriteComponent* sprite);

    void RegisterMesh(MeshComponent* mesh);
    void DeregisterMesh(MeshComponent* mesh);
    
    void Shutdown();
    bool resized = false;
private:
    bool InitializeBuffers();
    bool InitializePipelines(Window* window, SDL_GPUShader* vertexShader, SDL_GPUShader* fragmentShader);
    bool InitializeSamplers();
    bool CreateDepthStencils(Window* window);

    DrawInfo UploadVertices(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);

    SDL_GPUTextureFormat GetDepthStencilFormat();
    bool RenderTexture(const Texture* texture);

    SDL_GPUDevice* device = nullptr;
    SDL_GPUSampler* defaultSampler = nullptr;
    SDL_GPUTexture* defaultDepthStencil = nullptr;

    SDL_GPUBuffer* vertexBuffer = nullptr;
    SDL_GPUBuffer* indexBuffer = nullptr;
    uint32_t vertexBufferOffset = 0;
    uint32_t indexBufferOffset = 0;

    std::unordered_map<const Texture*, std::vector<SpriteComponent*>> spriteTextures;
    std::unordered_map<const Mesh*, std::vector<MeshComponent*>> meshes;

    std::unordered_map<const Mesh*, DrawInfo> meshDrawInfo;
    std::unordered_map<const Texture*, DrawInfo> spriteDrawInfo;

    std::unordered_map<PIPELINE_TYPE, SDL_GPUGraphicsPipeline*> pipelines;

    static std::unique_ptr<ShaderManager> shaderManager;
};