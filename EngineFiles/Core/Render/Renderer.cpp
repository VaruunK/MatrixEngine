#include "Renderer.hpp"
#include "Engine.hpp"
#include "World/Level/Level.hpp"
#include "Entity/Entity.hpp"
#include "Entity/Component/SpriteComponent/SpriteComponent.hpp"
#include "Core/WindowManager/Window/Window.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <span>
// #include <iostream>

std::unique_ptr<ShaderManager>  Renderer::shaderManager = nullptr;

Renderer::Renderer(SDL_GPUDevice* device) {
    this->device = device;
}

void Renderer::Render() {
    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);

    if (!commandBuffer) {
        SDL_Log("Failed to aquire Command Buffer: %s", SDL_GetError());
        return;
    }

    for (Window* window : Engine::GetEngine().GetWindowManager().GetWindows()) {
        SDL_GPUTexture* swapChainTexture;
        window->WaitAndAquireGPUGwapchainTexture(commandBuffer, &swapChainTexture, nullptr, nullptr);

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

            int windowWidth, windowHeight;
            window->GetWindowSize(&windowWidth, &windowHeight);
            float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

            auto projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
            auto view = glm::lookAt(glm::vec3(0, 0, 2), glm::vec3(0), glm::vec3(0, 1, 0));

            for (auto& [texture, sprites] : spriteTextures) {
                SDL_GPUTextureSamplerBinding binding{ texture->texture, defaultSampler };
                SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);

                for (auto* sprite : sprites) {
                    auto model = sprite->GetModelMatrix(aspectRatio);
                    auto mvp = projection * view * model;
                    SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvp, sizeof(mvp));
                    SDL_DrawGPUIndexedPrimitives(pass, 6, 1, 0, 0, 0);
                }
            }

            SDL_EndGPURenderPass(pass);
        }
    }
    
    if (!SDL_SubmitGPUCommandBuffer(commandBuffer)) {
        SDL_Log("Failed to submit Command Buffer: %s", SDL_GetError());
    }
}

bool Renderer::Initialize() {
    ShaderManager* rawSM = new ShaderManager(device);
    if (!rawSM) {
        SDL_Log("Failed to create Shader Manager");
        return false;
    }
    shaderManager.reset(rawSM);

    std::string vertShader = "shaders/TexturedQuadWithMatrix.vert.hlsl";
    std::string fragShader = "shaders/TexturedQuad.frag.hlsl";
    
    ShaderOptions optionsFrag = {
        .num_samplers = 1,
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 0
    };

    ShaderOptions optionsVert = {
        .num_samplers = 0,
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 1
    };

    SDL_GPUShader* vert = shaderManager.get()->LoadShader(vertShader, &optionsVert);
    SDL_GPUShader* frag = shaderManager.get()->LoadShader(fragShader, &optionsFrag);

    for (Window* window : Engine::GetEngine().GetWindowManager().GetWindows()) {
        if (!InitializePipelines(window, vert, frag)) {
            SDL_Log("Failed to create Pipelines");
            return false;
        }
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

    if (!InitializeSamplers()) {
        SDL_Log("Failed to create Samplers");
        return false;
    }

    if (!InitializeBuffers(vertices, indices)) {
        SDL_Log("Failed to create Buffers");
        return false;
    }

    return true;
}

bool Renderer::InitializeBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
    SDL_GPUBufferCreateInfo vertexInfo{};
    vertexInfo.size = vertices.size() * sizeof(Vertex);
    vertexInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;

    vertexBuffer = SDL_CreateGPUBuffer(device, &vertexInfo);
    if (!vertexBuffer) { 
        SDL_Log("Failed to create vertex buffer!"); 
        return false;
    }
    
    SDL_SetGPUBufferName(device, vertexBuffer, "Vertex Buffer");
    vertexBufferSize = vertexInfo.size;

    SDL_GPUBufferCreateInfo indexInfo{};
    indexInfo.size = indices.size() * sizeof(uint32_t);
    indexInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;

    indexBuffer = SDL_CreateGPUBuffer(device, &indexInfo);
    if (!indexBuffer) { 
        SDL_Log("Failed to create index buffer!"); 
        return false;
    }
    
    SDL_SetGPUBufferName(device, indexBuffer, "Index Buffer");
    indexBufferSize = indexInfo.size;

    SDL_GPUTransferBufferCreateInfo transferInfo{};
    transferInfo.size = vertexInfo.size + indexInfo.size;
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    
    auto* transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);
    if (!transferBuffer) { 
        SDL_Log("Failed to create transfer buffer!"); 
        return false;
    }

    auto* ptr = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(device, transferBuffer, false));
    if (!ptr) { 
        SDL_Log("Failed to map transfer buffer!"); 
        return false; 
    }

    std::memcpy(ptr, vertices.data(), vertexInfo.size);
    std::memcpy(ptr + vertexInfo.size, indices.data(), indexInfo.size);

    SDL_UnmapGPUTransferBuffer(device, transferBuffer);

    // upload immediately
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);

    SDL_GPUTransferBufferLocation src{ transferBuffer, 0 };
    SDL_GPUBufferRegion dst{ vertexBuffer, 0, vertexInfo.size };
    SDL_UploadToGPUBuffer(copyPass, &src, &dst, false);

    src.offset = vertexInfo.size;
    dst = { indexBuffer, 0, indexInfo.size };
    SDL_UploadToGPUBuffer(copyPass, &src, &dst, false);

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_ReleaseGPUTransferBuffer(device, transferBuffer);

    return true;
}

bool Renderer::RenderTexture(const Texture* texture) {
    int w = texture->data->w;
    int h = texture->data->h;
    uint32_t size = w * 4 * h;

    uint32_t alignedSize = (size + 255) & ~255;

    SDL_GPUTransferBufferCreateInfo transferInfo{};
    transferInfo.size = alignedSize;
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    
    auto* textureTransferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);
    if (!textureTransferBuffer) { 
        SDL_Log("Failed to create texture transfer buffer"); 
        return false; 
    }

    auto* ptr = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(device, textureTransferBuffer, false));
    
    if (!ptr) { 
        SDL_Log("Failed to map texture transfer buffer"); 
        return false; 
    }

    std::memcpy(ptr, texture->data->pixels, size);
    SDL_UnmapGPUTransferBuffer(device, textureTransferBuffer);

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);

    SDL_GPUTextureTransferInfo transferInfo2{
        .transfer_buffer = textureTransferBuffer,
        .offset = 0,
        .pixels_per_row = (uint32_t)w,
        .rows_per_layer = (uint32_t)h
    };

    SDL_GPUTextureRegion region{
        .texture = texture->texture,
        .mip_level = 0,
        .layer = 0,
        .x = 0,
        .y = 0,
        .z = 0,
        .w = (uint32_t)w,
        .h = (uint32_t)h,
        .d = 1
    };

    SDL_UploadToGPUTexture(copyPass, &transferInfo2, &region, false);

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_ReleaseGPUTransferBuffer(device, textureTransferBuffer);

    return true;
}

bool Renderer::InitializePipelines(Window* window, SDL_GPUShader* vertexShader, SDL_GPUShader* fragmentShader) {

    SDL_GPUColorTargetDescription colorTargetDescription{};
    colorTargetDescription.format = window->GetGPUSwapchainTextureFormat();
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

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineCreateInfo);

    if (!pipeline) {
        SDL_Log("Failed to create graphics pipeline: %s", SDL_GetError());
        return false;
    }

    SDL_Log("Pipeline created successfully!");
    pipelines[PIPELINE_TYPE::FILL_PIPELINE] = pipeline;

    return true;
}

bool Renderer::InitializeSamplers() {
    SDL_GPUSamplerCreateInfo samplerCreateInfo = {
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
    };

    defaultSampler = SDL_CreateGPUSampler(device, &samplerCreateInfo);

    if (!defaultSampler) {
        SDL_Log("Failed to create default sampler, %s", SDL_GetError());
        return false;
    }
    return true;
}

void Renderer::RegisterSprite(SpriteComponent* sprite) {
    const Texture* texture = sprite->GetTexture();
    if (!texture) return;

    if (!spriteTextures.contains(texture)) {
        RenderTexture(texture);
    }

    spriteTextures[texture].push_back(sprite);
}

void Renderer::DeregisterSprite(SpriteComponent* sprite) {
    const Texture* texture = sprite->GetTexture();
    if (!texture) return;

    auto it = spriteTextures.find(texture);
    if (it == spriteTextures.end()) return;

    auto& vec = it->second;
    vec.erase(std::ranges::find(vec, sprite));

    if (vec.empty()) {
        spriteTextures.erase(it);
    }
}

void Renderer::Shutdown() {
    if (device) {
        SDL_WaitForGPUIdle(device);
    }

    for (auto& [key, pipeline] : pipelines) {
        if (pipeline) {
            SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
        }
    }
    pipelines.clear();

    if (shaderManager) {
        shaderManager->Shutdown();
        shaderManager.reset();
    }
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