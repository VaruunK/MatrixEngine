#pragma once
#include "Core/ShaderManager/ShaderManager.hpp"
#include "Core/Structs/AssetStructs.hpp"
#include "Core/Enums/PipelineEnums.hpp"
#include "Core/Structs/RenderStructs.hpp"
#include "Core/Structs/FrameData.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

class SpriteComponent;
class MeshComponent;

struct FrameData;
struct Appstate;

class WorldRenderer {
    friend class EditorRenderer;
public:
    WorldRenderer(Appstate& appstate);
    ~WorldRenderer();

    bool Initialize();
    void Render(FrameData& frame);
    void RenderAndSubmit(FrameData& frame);

    void RegisterSprite(SpriteComponent* sprite);
    void DeregisterSprite(SpriteComponent* sprite);

    void RegisterMesh(MeshComponent* mesh);
    void DeregisterMesh(MeshComponent* mesh);

    bool resized = false;

private:
    bool InitializeBuffers();
    bool InitializePipelines(SDL_GPUShader* vertexShader, SDL_GPUShader* fragmentShader);
    bool InitializeSamplers();
    bool CreateDepthStencil();
    bool CreateMSAATexture();
    bool CreateOffscreenTexture();

    DrawInfo UploadVertices(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);

    SDL_GPUTextureFormat GetDepthStencilFormat();
    bool RenderTexture(const Texture* texture);

    Appstate& appstate;

    SDL_GPUSampler* defaultSampler = nullptr;
    SDL_GPUTexture* depthStencilTexture = nullptr;
    SDL_GPUTexture* msaaTexture = nullptr;    

    SDL_GPUBuffer* vertexBuffer = nullptr;
    SDL_GPUBuffer* indexBuffer = nullptr;
    uint32_t vertexBufferOffset = 0;
    uint32_t indexBufferOffset = 0;

    SDL_GPUSampleCount sampleCount = SDL_GPU_SAMPLECOUNT_8;

    std::unordered_map<const Texture*, std::vector<SpriteComponent*>> spriteTextures;
    std::unordered_map<const Mesh*, std::vector<MeshComponent*>> meshes;

    std::unordered_map<const Mesh*, DrawInfo> meshDrawInfo;
    std::unordered_map<const Texture*, DrawInfo> spriteDrawInfo;

    std::unordered_map<PIPELINE_TYPE, SDL_GPUGraphicsPipeline*> pipelines;

    static std::unique_ptr<ShaderManager> shaderManager;

    SDL_GPUTexture* offscreenTexture = nullptr;
    SDL_GPUSampler* offscreenSampler = nullptr;
    bool offscreenTextureDirty = true;

    bool msaaEnabled = true;
};