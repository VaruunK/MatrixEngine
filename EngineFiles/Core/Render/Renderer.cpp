#include "Renderer.hpp"
#include "World/Level/Level.hpp"
#include "Entity/Entity.hpp"
//#include "Entity/Component/ImageComponent/ImageComponent.hpp"
#include <SDL3_shadercross/SDL_shadercross.h>
#include <iostream>
#include <span>

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

    SDL_Surface* icon = IMG_Load(iconFilePath.c_str());

    if (!icon) {
        SDL_Log("couldn't load icon: %s", SDL_GetError());
    }
    else {
        SDL_SetWindowIcon(window.get(), icon);
        SDL_DestroySurface(icon);
    }

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
        SDL_GPUColorTargetInfo colorTarget{};
        colorTarget.texture = swapChainTexture;
        colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTarget.store_op = SDL_GPU_STOREOP_STORE;
        colorTarget.clear_color = { 0.1f, 0.1f, 0.1f, 1.0f };

        std::vector<SDL_GPUColorTargetInfo> colorTargets{ colorTarget };

        SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(commandBuffer, colorTargets.data(), colorTargets.size(), nullptr);
        SDL_BindGPUGraphicsPipeline(pass, pipelines[PIPELINE_TYPE::FILL_PIPELINE]);

        std::vector<SDL_GPUBufferBinding> vertexBinding{ { vertexBuffer, 0 } };
        SDL_BindGPUVertexBuffers(pass, 0, vertexBinding.data(), vertexBinding.size());

        SDL_GPUBufferBinding indexBufferBinding{ indexBuffer, 0 };
        SDL_BindGPUIndexBuffer(pass, &indexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

        SDL_BindGPUFragmentSamplers(pass, 0, textureSamplerBindings.data(), textureSamplerBindings.size());

        SDL_DrawGPUIndexedPrimitives(pass, indices.size(), 1, 0, 0, 0);

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
        SDL_Log("Failed to create device: %s", SDL_GetError());
        return false;
    }

    if (!CreateRenderWindow(name.c_str(), initWinSizeX, initWinSizeY)) {
        SDL_Log("Failed to create render window: %s", SDL_GetError());
        return false;
    }

    if (!SDL_ClaimWindowForGPUDevice(device.get(), window.get())) {
        SDL_Log("Failed to claim window: %s", SDL_GetError());
        return false;
    }

    ShaderManager* rawSM = new ShaderManager(device.get());
    if (!rawSM) {
        SDL_Log("Failed to create Shader Manager");
        return false;
    }
    shaderManager.reset(rawSM);

    std::string vertShader = "shaders/TexturedQuad.vert.hlsl";
    std::string fragShader = "shaders/TexturedQuad.frag.hlsl";
    std::string texture = "Content/nanodsa.png";

    ShaderOptions options = {
        .num_samplers = 1,
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 0
    };

    SDL_GPUShader* vert = shaderManager.get()->LoadShader(vertShader);
    SDL_GPUShader* frag = shaderManager.get()->LoadShader(fragShader, &options);

    if (!InitializePipelines(vert, frag)) {
        SDL_Log("Failed to create Pipelines");
        return false;
    }

    vertices = {
        { { -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } },
        { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } },
        { {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f } },
        { { -0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f } },
    };

    indices = {
        0, 1, 2,
        0, 2, 3,
    };

    if (!InitializeBuffers(vertices, indices, texture)) {
        SDL_Log("Failed to create Buffers");
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
    colorTargetDescription.blend_state.enable_blend = true;
    colorTargetDescription.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    colorTargetDescription.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    colorTargetDescription.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    colorTargetDescription.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    colorTargetDescription.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    colorTargetDescription.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
    
    std::vector colorTargetDescriptions{ colorTargetDescription };

    SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
    targetInfo.color_target_descriptions = colorTargetDescriptions.data();
    targetInfo.num_color_targets = colorTargetDescriptions.size();

    std::vector<SDL_GPUVertexAttribute> vertexAttributes{};
    std::vector<SDL_GPUVertexBufferDescription> vertexBufferDescriptions{};

    vertexAttributes.emplace_back(0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(Vertex, position));
    vertexAttributes.emplace_back(1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(Vertex, uv));

    vertexBufferDescriptions.emplace_back(0, sizeof(Vertex), SDL_GPU_VERTEXINPUTRATE_VERTEX, 0);

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
    pipelineCreateInfo.vertex_input_state = vertexInputState;

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device.get(), &pipelineCreateInfo);

    if (!pipeline) {
        SDL_Log("Failed to create graphics pipeline: %s", SDL_GetError());
        return false;
    }

    SDL_Log("Pipeline created successfully!");
    pipelines[PIPELINE_TYPE::FILL_PIPELINE] = pipeline;

    return true;
}

bool Renderer::InitializeBuffers(std::vector<Vertex> vertices, std::vector<Uint32> indices, std::string& texturePath)
{
    SDL_GPUBufferCreateInfo vertexBufferCreateInfo{};
    vertexBufferCreateInfo.size = vertices.size() * sizeof(Vertex);
    vertexBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    vertexBuffer = SDL_CreateGPUBuffer(device.get(), &vertexBufferCreateInfo);

    if (!vertexBuffer) {
        SDL_Log("Failed to create vertex buffer!");
        return false;
    }

    SDL_SetGPUBufferName(device.get(), vertexBuffer, "Vertex Buffer");

    SDL_GPUBufferCreateInfo indexBufferCreateInfo{};
    indexBufferCreateInfo.size = indices.size() * sizeof(Uint32);
    indexBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    indexBuffer = SDL_CreateGPUBuffer(device.get(), &indexBufferCreateInfo);

    if (!indexBuffer) {
        SDL_Log("Failed to create index buffer!");
        return false;
    }

    SDL_SetGPUBufferName(device.get(), indexBuffer, "Index Buffer");

    SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo{};
    transferBufferCreateInfo.size = vertexBufferCreateInfo.size + indexBufferCreateInfo.size;
    transferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    auto transferBuffer = SDL_CreateGPUTransferBuffer(device.get(), &transferBufferCreateInfo);

    if (!transferBuffer) {
        SDL_Log("Failed to create transfer buffer!");
        return false;
    }

    auto transferBufferDataPtr = static_cast<Uint8*>(SDL_MapGPUTransferBuffer(device.get(), transferBuffer, false));
    if (!transferBufferDataPtr) {
        SDL_Log("Failed to create transfer buffer!");
        return false;
    }

    std::span vertexBufferData{
        reinterpret_cast<Vertex*>(transferBufferDataPtr),
        vertices.size()
    };

    std::ranges::copy(vertices, vertexBufferData.begin());

    std::span indexBufferData{
        reinterpret_cast<Uint32*>(transferBufferDataPtr + vertexBufferCreateInfo.size),
        indices.size()
    };

    std::ranges::copy(indices, indexBufferData.begin());

    SDL_UnmapGPUTransferBuffer(device.get(), transferBuffer);

    Texture* texture = CreateTexture(device.get(), texturePath);
    if (!texture) {
        SDL_Log("Failed to create texture");
        return false;
    }
    SDL_Surface* imageData = texture->data;

    SDL_GPUTransferBufferCreateInfo textureTransferBufferCreateInfo{};
    textureTransferBufferCreateInfo.size = imageData->w * 4 * imageData->h;
    textureTransferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;

    auto textureTransferBuffer = SDL_CreateGPUTransferBuffer(device.get(), &textureTransferBufferCreateInfo);
    if (!textureTransferBuffer) {
        SDL_Log("Failed to create transfer buffer");
        return false;
    }

    auto textureTransferBufferDataPtr = static_cast<Uint8*>(SDL_MapGPUTransferBuffer(device.get(), textureTransferBuffer, false));
    if (!textureTransferBufferDataPtr) {
        SDL_Log("Failed to map transfer buffer");
        return false;
    }

    std::memcpy(textureTransferBufferDataPtr, imageData->pixels, textureTransferBufferCreateInfo.size);

    SDL_UnmapGPUTransferBuffer(device.get(), textureTransferBuffer);

    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device.get());

    if (!commandBuffer) {
        SDL_Log("Failed to aquire Command Buffer: %s", SDL_GetError());
        return false;
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

    SDL_GPUTransferBufferLocation source{};
    SDL_GPUBufferRegion destination{};

    source.transfer_buffer = transferBuffer;
    source.offset = 0;

    destination.buffer = vertexBuffer;
    destination.size = vertexBufferCreateInfo.size;
    destination.offset = 0;

    SDL_UploadToGPUBuffer(copyPass, &source, &destination, false);

    source.offset = vertexBufferCreateInfo.size;
    destination.buffer = indexBuffer;
    destination.offset = 0;
    destination.size = indexBufferCreateInfo.size;

    SDL_UploadToGPUBuffer(copyPass, &source, &destination, false);

    SDL_GPUTextureTransferInfo textureTransferInfo{};
    textureTransferInfo.transfer_buffer = textureTransferBuffer;
    textureTransferInfo.pixels_per_row = imageData->w;
    textureTransferInfo.rows_per_layer = imageData->h;

    SDL_GPUTextureRegion textureRegion{};
    textureRegion.texture = texture->texture;
    textureRegion.x = 0;
    textureRegion.y = 0;
    textureRegion.z = 0;
    textureRegion.w = static_cast<Uint32>(imageData->w);
    textureRegion.h = static_cast<Uint32>(imageData->h);
    textureRegion.d = 1;

    SDL_UploadToGPUTexture(copyPass, &textureTransferInfo, &textureRegion, false);

    SDL_EndGPUCopyPass(copyPass);

    if (!SDL_SubmitGPUCommandBuffer(commandBuffer)) {
        SDL_Log("Failed to submit Command Buffer: %s", SDL_GetError());
        return false;
    }

    SDL_ReleaseGPUTransferBuffer(device.get(), transferBuffer);
    SDL_ReleaseGPUTransferBuffer(device.get(), textureTransferBuffer);
    BindTextureToSampler(device.get(), texture);
    return true;
}

Texture* Renderer::CreateTexture(SDL_GPUDevice* device, std::string& texturePath) {
    SDL_Surface* imageData = SDL_LoadSurface(texturePath.c_str());

    if (!imageData) {
        SDL_Log("Failed to load image data: %s", SDL_GetError());
        return nullptr;
    }

    SDL_Surface* converted = SDL_ConvertSurface(imageData, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(imageData);
    imageData = converted;

    SDL_GPUTextureCreateInfo textureCreateInfo{
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
        .width = static_cast<Uint32>(imageData->w),
        .height = static_cast<Uint32>(imageData->h),
        .layer_count_or_depth = 1,
        .num_levels = 1,
    };

    SDL_GPUTexture* texture = SDL_CreateGPUTexture(device, &textureCreateInfo);
    if (!texture) {
        SDL_Log("Failed to create Texture");
        return nullptr;
    }
    SDL_SetGPUTextureName(device, texture, texturePath.c_str());

    Texture* newTexture = new Texture;
    newTexture->texture = texture;
    newTexture->data = imageData;

    return newTexture;
}

void Renderer::BindTextureToSampler(SDL_GPUDevice* device, Texture* texture) {
    SDL_GPUSamplerCreateInfo samplerCreateInfo = {
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
    };

    SDL_GPUSampler* sampler = SDL_CreateGPUSampler(device, &samplerCreateInfo);

    SDL_GPUTextureSamplerBinding textureSamplerBinding{};
    textureSamplerBinding.texture = texture->texture;
    textureSamplerBinding.sampler = sampler;

    textureSamplerBindings.push_back(textureSamplerBinding);
}

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