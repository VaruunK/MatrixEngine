#pragma once
#include "Core/ShaderManager/ShaderManager.hpp"
#include "PipelineEnums.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

struct Texture;
struct Vertex;
struct Mesh;
struct SDL_GPUDevice;
struct SDL_GPUShader;
struct SDL_GPUSampler;
struct SDL_GPUTexture;
struct SDL_GPUBuffer;
class SpriteComponent;
class MeshComponent;
class Window;

class Renderer {
public:
    Renderer(SDL_GPUDevice* device);
    bool Initialize();
    SDL_GPUDevice* GetGPUDevice();
    void Render();

    void RegisterSprite(SpriteComponent* sprite);
    void DeregisterSprite(SpriteComponent* sprite);

    void RegisterMesh(MeshComponent* mesh);
    void DeregisterMesh(MeshComponent* mesh);

    Texture* CreateTexture(const std::string &texturFilePath);
    
    void Shutdown();

private:
    bool InitializeBuffers();
    bool InitializePipelines(Window* window, SDL_GPUShader* vertexShader, SDL_GPUShader* fragmentShader);
    bool InitializeSamplers();
    bool InitializeDepthStencils();

    bool UploadVertices(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);

    SDL_GPUTextureFormat GetDepthStencilFormat();
    bool RenderTexture(const Texture* texture);

    SDL_GPUDevice* device = nullptr;
    SDL_GPUSampler* defaultSampler = nullptr;
    SDL_GPUTexture* defaultDepthStencil = nullptr;

    SDL_GPUBuffer* vertexBuffer = nullptr;
    SDL_GPUBuffer* indexBuffer = nullptr;

    std::unordered_map<const Texture*, std::vector<SpriteComponent*>> spriteTextures;
    std::unordered_map<const Mesh*, std::vector<MeshComponent*>> meshes;

    std::unordered_map<PIPELINE_TYPE, SDL_GPUGraphicsPipeline*> pipelines;

    static std::unique_ptr<ShaderManager> shaderManager;
};