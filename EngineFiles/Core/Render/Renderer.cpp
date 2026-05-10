#include "Renderer.hpp"
#include "World/Level/Level.hpp"
#include "Entity/Entity.hpp"
//#include "Entity/Component/ImageComponent/ImageComponent.hpp"
#include <SDL3_shadercross/SDL_shadercross.h>
#include <glm/glm.hpp>
#include <iostream>

std::unique_ptr<SDL_Window, SDLWindowDeleter> Renderer::window = nullptr;
std::unique_ptr<SDL_GPUDevice, SDLGPUDeviceDeleter> Renderer::device = nullptr;
std::unique_ptr<ShaderManager>  Renderer::shaderManager = nullptr;


bool Renderer::Initialize(const std::string& name, int initWinSizeX, int initWinSizeY) {
    return InitializeSDLWindow(name, initWinSizeX, initWinSizeY);
}

bool Renderer::Initialize(const std::string& name, const std::string& iconFilePath, int initWinSizeX, int initWinSizeY) {

    if (!InitializeSDLWindow(name, initWinSizeX, initWinSizeY)) {
        return false;
    }

    //SDL_Surface* icon = IMG_Load(iconFilePath.c_str());

    /*if (!icon) {
        SDL_Log("couldn't load icon: ", SDL_GetError());
    } else {
        SDL_SetWindowIcon(rawWindow, icon);
        SDL_DestroySurface(icon);
    }*/
    
    return true;
}


void Renderer::SetWindowType() {
    
}

void Renderer::Render() {
    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device.get());

    if (!commandBuffer) {
        SDL_Log("Failed to aquire Command Buffer: %s", SDL_GetError());
        return;
    }

    SDL_GPUTexture* swapChainTexture;
    SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window.get(), &swapChainTexture, nullptr, nullptr);

    if (swapChainTexture) {
        SDL_GPUColorTargetInfo colorTarget {};
        colorTarget.texture = swapChainTexture;
        colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTarget.store_op = SDL_GPU_STOREOP_STORE;
        colorTarget.clear_color = { 0.1f, 0.1f, 0.1f, 1.0f };

        std::vector<SDL_GPUColorTargetInfo> colorTargets{ colorTarget };

        SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(commandBuffer, colorTargets.data(), colorTargets.size(), nullptr);
        SDL_BindGPUGraphicsPipeline(pass, pipelines[PIPELINE_TYPE::FILL_PIPELINE]);
        SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);

        SDL_EndGPURenderPass(pass);
    }
    
    if (!SDL_SubmitGPUCommandBuffer(commandBuffer)) {
        SDL_Log("Failed to submit Command Buffer: %s", SDL_GetError());
    }
}

bool Renderer::InitializeSDLWindow(const std::string& name, int initWinSizeX, int initWinSizeY) {
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
            return false;
        }
    }

    if (!CreateDevice()) {
        return false;
    }

    if (!CreateRenderWindow(name.c_str(), initWinSizeX, initWinSizeY)) {
        return false;
    }

    if (!SDL_ClaimWindowForGPUDevice(device.get(), window.get())) {
        SDL_Log("Failed to claim window: %s", SDL_GetError());
        return false;
    }

    ShaderManager* rawSM = new ShaderManager(device.get());
    if (!rawSM) {
        SDL_Log("Failed to create Shader Manaager");
        return false;
    }

    shaderManager.reset(rawSM);

    std::string fragShader = "C:/development/Engine/shaders/SolidColor.frag.hlsl";
    std::string vertShader = "C:/development/Engine/shaders/RawTriangle.vert.hlsl";

    if (!InitializePipelines(shaderManager.get()->LoadShader(vertShader), shaderManager.get()->LoadShader(fragShader))) {
        return false;
    }

    return true;
}

bool Renderer::CreateRenderWindow(const std::string& name, int initWinSizeX, int initWinSizeY) {
    SDL_Window* rawWindow = SDL_CreateWindow(name.c_str(), initWinSizeX, initWinSizeY, SDL_WINDOW_RESIZABLE);

    if (!rawWindow) {
        SDL_Log("Failed to create Window: %s", SDL_GetError());
        return false;
    }

    window.reset(rawWindow);
    return true;
}

bool Renderer::CreateDevice() {
    SDL_GPUDevice* rawDevice = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
        true,
        nullptr
    );

    if (!rawDevice) {
        SDL_Log("Failed to create GPU device: %s", SDL_GetError());
        return false;
    }

    device.reset(rawDevice);
    return true;
}

bool Renderer::InitializePipelines(SDL_GPUShader* vertexShader, SDL_GPUShader* fragmentShader) {

    SDL_GPUColorTargetDescription colorTargetDescription{};
    colorTargetDescription.format = SDL_GetGPUSwapchainTextureFormat(device.get(), window.get());
    std::vector colorTargetDescriptions{ colorTargetDescription };

    SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
    targetInfo.color_target_descriptions = colorTargetDescriptions.data();
    targetInfo.num_color_targets = colorTargetDescriptions.size();

    std::vector<SDL_GPUVertexAttribute> vertexAttributes{};
    std::vector<SDL_GPUVertexBufferDescription> vertexBufferDescriptions{};

    vertexAttributes.emplace_back(0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 0);
    vertexAttributes.emplace_back(1, 0, SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM, sizeof(float) * 3);

    vertexBufferDescriptions.emplace_back(0, sizeof(glm::vec3), SDL_GPU_VERTEXINPUTRATE_VERTEX, 0);

    SDL_GPUVertexInputState vertexInputState{};
    vertexInputState.vertex_attributes = vertexAttributes.data();
    vertexInputState.num_vertex_attributes = vertexAttributes.size();
    vertexInputState.vertex_buffer_descriptions = vertexBufferDescriptions.data();
    vertexInputState.num_vertex_buffers = vertexBufferDescriptions.size();

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.vertex_shader = vertexShader;
    pipelineCreateInfo.fragment_shader = fragmentShader;
    pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    pipelineCreateInfo.target_info = targetInfo;

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device.get(), &pipelineCreateInfo);

    if (!pipeline) {
        SDL_Log("Failed to create graphics pipeline!");
        return false;
    }

    SDL_Log("Pipeline created successfully!");
    pipelines[PIPELINE_TYPE::FILL_PIPELINE] = pipeline;
    return true;
}

//bool Renderer::InitializePipelines(SDL_GPUShader* vertexShader, SDL_GPUShader* fragmentShader) {
//    SDL_GPUColorTargetDescription colorTargetDescription{};
//    colorTargetDescription.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
//    colorTargetDescription.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
//    colorTargetDescription.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
//    colorTargetDescription.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
//    colorTargetDescription.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
//    colorTargetDescription.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
//    colorTargetDescription.blend_state.color_write_mask = SDL_GPU_COLORCOMPONENT_R |
//        SDL_GPU_COLORCOMPONENT_G |
//        SDL_GPU_COLORCOMPONENT_B |
//        SDL_GPU_COLORCOMPONENT_A;
//    colorTargetDescription.blend_state.enable_blend = false;
//    colorTargetDescription.format = SDL_GetGPUSwapchainTextureFormat(device.get(), window.get());
//
//    vector colorTargetDescriptions{ colorTargetDescription };
//
//    SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
//    targetInfo.color_target_descriptions = colorTargetDescriptions.data();
//    targetInfo.num_color_targets = colorTargetDescriptions.size();
//
//    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo{};
//    pipelineCreateInfo.vertex_shader = vertexShader;
//    pipelineCreateInfo.fragment_shader = fragmentShader;
//    pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
//    pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
//    //pipelineCreateInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
//    pipelineCreateInfo.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
//    pipelineCreateInfo.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;
//    pipelineCreateInfo.multisample_state.sample_mask = 0;
//    pipelineCreateInfo.target_info = targetInfo;
//
//    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device.get(), &pipelineCreateInfo);
//
//    if (!pipeline) {
//        SDL_Log("Failed to create graphics pipeline!");
//        return false;
//    }
//
//    SDL_Log("Pipeline created successfully!");
//    pipelines[PIPELINE_TYPE::FILL_PIPELINE] = pipeline;
//    return true;
//}

//void Renderer::Render() {
//    if (!renderer) return;
//
//    for (ImageComponent *ic : imageComponents) {
//        if (!ic || !ic->IsVisible() || ic->GetTextureFilePath().empty()) continue;
//
//        SDL_Texture* texture = ic->GetTexture();
//
//        if (!texture) continue;
//
//        float w, h;
//
//        if (!SDL_GetTextureSize(texture, &w, &h)) {
//            cerr << "Failed to get texture size: " << SDL_GetError() << endl;
//            continue;
//        }
//
//        SDL_FRect dstRect = {
//            ic->GetOwner()->GetXPos(),
//            ic->GetOwner()->GetYPos(),
//            w * ic->GetScaleX(),
//            h * ic->GetScaleY()
//        };
//
//        if (!SDL_RenderTexture(renderer.get(), texture, nullptr, &dstRect)) {
//            cout << "Failed to render : " << SDL_GetError() << endl;
//        }
//    }
//}

//void Renderer::Clear() {
//    if (!renderer.get()) return;
//    SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);
//    SDL_RenderClear(renderer.get());
//}
//
//void Renderer::Present() {
//    if (!renderer.get()) return;
//    SDL_RenderPresent(renderer.get());
//}

//void Renderer::RegisterImageComponent(ImageComponent* imageComponent) {
//    imageComponents.push_back(imageComponent);
//    const string& filepath = imageComponent->GetTextureFilePath();
//    if (!filepath.empty()) {
//        GetOrLoadTexture(filepath);
//    }
//}
//
//void Renderer::DeregisterImageComponent(ImageComponent* imageComponent)
//{
//}

//SDL_Texture* Renderer::GetOrLoadTexture(const string& filepath) {
//    auto it = textureCache.find(filepath);
//    if (it != textureCache.end()) {
//        return it->second.get();
//    }
//
//    SDL_Texture* raw = IMG_LoadTexture(renderer.get(), filepath.c_str());
//    if (!raw) {
//        cerr << "Failed to load texture: " << filepath
//            << " - " << SDL_GetError() << endl;
//        return nullptr;
//    }
//
//    auto texture = unique_ptr<SDL_Texture, SDLTextureDeleter>(raw);
//    SDL_Texture* result = texture.get();
//    textureCache.emplace(filepath, move(texture));
//    return result;
//}
//
//bool Renderer::PreloadTexture(const string& filepath) {
//    return GetOrLoadTexture(filepath) != nullptr;
//}
//
//void Renderer::ClearTextureCache() {
//    textureCache.clear();
//}

void Renderer::Shutdown() {
    if (device) {
        SDL_WaitForGPUIdle(device.get());
    }

    if (device && window) {
        SDL_ReleaseWindowFromGPUDevice(device.get(), window.get());
    }

    for (auto& [key, pipeline] : pipelines) {
        if (pipeline) {
            SDL_ReleaseGPUGraphicsPipeline(device.get(), pipeline);
        }
    }
    pipelines.clear();

    if (shaderManager) {
        shaderManager->Shutdown();
        shaderManager.reset();
    }

    device.reset();
    window.reset();
}