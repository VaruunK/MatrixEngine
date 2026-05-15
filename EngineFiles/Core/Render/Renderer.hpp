#pragma once

#include "Core/ShaderManager/ShaderManager.hpp"
#include "PipelineEnums.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <glm/glm.hpp>
#include <SDL3_image/SDL_image.h>

class ImageComponent;

struct SDLWindowDeleter {
    void operator()(SDL_Window* w) const {
        if (w) SDL_DestroyWindow(w);
    }
};

struct SDLGPUDeviceDeleter {
    void operator()(SDL_GPUDevice* d) const {
        if (d) SDL_DestroyGPUDevice(d);
    }
};

struct Vertex {
    glm::vec3 position;
    glm::vec2 uv;
};

struct Texture {
    SDL_Surface* data;
    SDL_GPUTexture* texture;
};

class Renderer {
public:  

    Renderer() = default;

    bool Initialize(const std::string& name, int initWinSizeX, int initWinSizeY);
    bool Initialize(const std::string& name, const std::string& iconFilePath, int initWinSizeX, int initWinSizeY);
    void Render();

    void RenderImage(std::vector<Vertex> vertices, std::vector<Uint32> indices, std::string& texturePath);

    void SetWindowType();
    //void Clear();
    //void Present();

    void Shutdown();

private:
    bool CreateRenderWindow(const std::string& name, int initWinSizeX, int initWinSizeY);
    bool InitializeSDLWindow(const std::string& name, int initWinSizeX, int initWinSizeY);
    bool InitializeBuffers(std::vector<Vertex> vertices, std::vector<Uint32> indices, std::string& texturePath);
    bool InitializePipelines(SDL_GPUShader* vetexShader, SDL_GPUShader* fragmentShader);
    Texture* CreateTexture(SDL_GPUDevice* device, std::string& texturePath);
    void BindTextureToSampler(SDL_GPUDevice* device, Texture* texture);
    bool CreateDevice();

    static std::unique_ptr<ShaderManager> shaderManager;
    static std::unique_ptr<SDL_Window, SDLWindowDeleter> window;
    static std::unique_ptr<SDL_GPUDevice, SDLGPUDeviceDeleter> device;

    std::vector<Vertex> vertices;
    std::vector<Uint32> indices;

    std::vector<SDL_GPUTextureSamplerBinding> textureSamplerBindings;

    SDL_GPUBuffer* vertexBuffer = nullptr;
    SDL_GPUBuffer* indexBuffer = nullptr;

    std::unordered_map<PIPELINE_TYPE, SDL_GPUGraphicsPipeline*> pipelines;
    
    Uint32 MAX_TRANSFER_SIZE = 1000;

    //vector<ImageComponent*> imageComponents;
};