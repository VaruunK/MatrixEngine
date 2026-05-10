#pragma once

#include "Core/ShaderManager/ShaderManager.hpp"
#include "PipelineEnums.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
//#include <SDL3_image/SDL_image.h>

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

class Renderer {
public:
    Renderer() = default;

    bool Initialize(const std::string& name, int initWinSizeX, int initWinSizeY);
    bool Initialize(const std::string& name, const std::string& iconFilePath, int initWinSizeX, int initWinSizeY);
    void Render();

    void SetWindowType();
    //void Clear();
    //void Present();

    void Shutdown();

private:

    bool InitializeSDLWindow(const std::string& name, int initWinSizeX, int initWinSizeY);
    bool InitializePipelines(SDL_GPUShader* vetexShader, SDL_GPUShader* fragmentShader);
    bool CreateRenderWindow(const std::string& name, int initWinSizeX, int initWinSizeY);
    bool CreateDevice();

    static std::unique_ptr<ShaderManager> shaderManager;
    static std::unique_ptr<SDL_Window, SDLWindowDeleter> window;
    static std::unique_ptr<SDL_GPUDevice, SDLGPUDeviceDeleter> device;

    std::unordered_map<PIPELINE_TYPE, SDL_GPUGraphicsPipeline*> pipelines;
    
    //vector<ImageComponent*> imageComponents;
};