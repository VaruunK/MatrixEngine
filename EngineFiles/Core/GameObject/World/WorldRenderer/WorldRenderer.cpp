#include "WorldRenderer.hpp"
#include "Core/Structs/RenderStructs.hpp"
#include "Core/Structs/View.hpp"
#include "Core/Structs/FrameData.hpp"
#include "Engine.hpp"
#include "Core/GameObject/Entity/Entity.hpp"
#include "Core/GameObject/Component/SpriteComponent/SpriteComponent.hpp"
#include "Core/GameObject/Component/MeshComponent/MeshComponent.hpp"
#include "Core/WindowManager/Window/Window.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
// #include <iostream>

std::unique_ptr<ShaderManager> WorldRenderer::shaderManager = nullptr;

WorldRenderer::WorldRenderer(SDL_GPUDevice* device) {
    this->device = device;
}

bool WorldRenderer::CreateOffscreenTexture(Window* window) {
    int w, h;
    window->GetWindowSize(&w, &h);

    SDL_GPUTextureCreateInfo info{};
    info.format = window->GetGPUSwapchainTextureFormat();
    info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
    info.width = static_cast<uint32_t>(w);
    info.height = static_cast<uint32_t>(h);
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1; // resolve MSAA into this

    offscreenTexture = SDL_CreateGPUTexture(device, &info);
    if (!offscreenTexture) {
        SDL_Log("Failed to create offscreen texture: %s", SDL_GetError());
        return false;
    }
    SDL_SetGPUTextureName(device, offscreenTexture, "Offscreen Render Texture");
    return true;
}

void WorldRenderer::Render(FrameData& frame) {

    Window* window = Engine::GetEngine().GetWindowManager().GetMainWindow();
    if (!offscreenTexture || !frame.commandBuffer) return;
    
    /* SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            resized = true;
        }
    }*/
    
    if (resized) {
        SDL_WaitForGPUIdle(device);

        SDL_ReleaseGPUTexture(device, depthStencilTexture);
        SDL_ReleaseGPUTexture(device, msaaTexture);

        CreateDepthStencil(window);
        CreateMSAATexture(window);

        resized = false;
    }

    SDL_GPUColorTargetInfo colorTarget{};
    colorTarget.clear_color = { 0.1f, 0.1f, 0.1f, 1.0f };
    colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;

    if (msaaEnabled) {
        colorTarget.texture = msaaTexture;
        colorTarget.store_op = SDL_GPU_STOREOP_RESOLVE;
        colorTarget.resolve_texture = offscreenTexture;
    } else {
        colorTarget.texture = offscreenTexture;
        colorTarget.store_op = SDL_GPU_STOREOP_STORE;
    }

    frame.viewportTexture = offscreenTexture;

    std::vector<SDL_GPUColorTargetInfo> colorTargets{ colorTarget };

    SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo{};
    depthStencilTargetInfo.clear_depth = 1.0f;
    depthStencilTargetInfo.clear_stencil = 0;
    depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    depthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;
    depthStencilTargetInfo.texture = depthStencilTexture;

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(frame.commandBuffer, colorTargets.data(), colorTargets.size(), &depthStencilTargetInfo);
    SDL_BindGPUGraphicsPipeline(pass, pipelines[PIPELINE_TYPE::FILL_PIPELINE]);

    std::vector<SDL_GPUBufferBinding> vertexBinding{ { vertexBuffer, 0 } };
    SDL_BindGPUVertexBuffers(pass, 0, vertexBinding.data(), vertexBinding.size());

    SDL_GPUBufferBinding indexBufferBinding{ indexBuffer, 0 };
    SDL_BindGPUIndexBuffer(pass, &indexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

    int windowWidth, windowHeight;
    window->GetWindowSize(&windowWidth, &windowHeight);
    float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

    for (auto& [mesh, components] : meshes) {
        if (!mesh->texture) continue;

        SDL_GPUTextureSamplerBinding binding{ mesh->texture->texture, defaultSampler };
        SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);

        const DrawInfo& di = meshDrawInfo[mesh];

        for (auto* component : components) {
            auto model = component->GetModelMatrix(aspectRatio);

            auto mvp = frame.view->projectionMatrix * frame.view->viewMatrix * model;
            SDL_PushGPUVertexUniformData(frame.commandBuffer, 0, &mvp, sizeof(mvp));
            SDL_DrawGPUIndexedPrimitives(pass, di.indexCount, 1, di.firstIndex, di.vertexOffset, 0);
        }
    }

    for (auto& [texture, sprites] : spriteTextures) {
        if (!texture->texture) continue;
        SDL_GPUTextureSamplerBinding binding{ texture->texture, defaultSampler };
        SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);

        const DrawInfo& di = spriteDrawInfo[texture];
        for (auto* sprite : sprites) {
            auto mvp = frame.view->projectionMatrix * frame.view->viewMatrix * sprite->GetModelMatrix(aspectRatio);
            SDL_PushGPUVertexUniformData(frame.commandBuffer, 0, &mvp, sizeof(mvp));
            SDL_DrawGPUIndexedPrimitives(pass, di.indexCount, 1, di.firstIndex, di.vertexOffset, 0);
        }
    }
    SDL_EndGPURenderPass(pass);
}

bool WorldRenderer::Initialize() {
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

    Window* window = Engine::GetEngine().GetWindowManager().GetMainWindow();
    
    if (!InitializePipelines(window, vert, frag)) {
        SDL_Log("Failed to create Pipelines");
        return false;
    }

    if (msaaEnabled) {
        if (!CreateMSAATexture(window)) {
            SDL_Log("Failed to create MSAA Texture");
            return false;
        }
    }

    if (!CreateDepthStencil(window)) {
        SDL_Log("Failed to create Depth Stencils");
        return false;
    }

    if (!CreateOffscreenTexture(window)) {
        SDL_Log("Failed to create offscreen texture");
        return false;
    }
    
    if (!InitializeSamplers()) {
        SDL_Log("Failed to create Samplers");
        return false;
    }

    if (!InitializeBuffers()) {
        SDL_Log("Failed to create Buffers");
        return false;
    }

    return true;
}

bool WorldRenderer::InitializeBuffers() {
    SDL_GPUBufferCreateInfo vertexInfo{};
    vertexInfo.size = 50000 * sizeof(Vertex);
    vertexInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;

    vertexBuffer = SDL_CreateGPUBuffer(device, &vertexInfo);
    if (!vertexBuffer) { 
        SDL_Log("Failed to create vertex buffer!"); 
        return false;
    }
    
    SDL_SetGPUBufferName(device, vertexBuffer, "Vertex Buffer");

    SDL_GPUBufferCreateInfo indexInfo{};
    indexInfo.size = 150000 * sizeof(uint32_t);
    indexInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;

    indexBuffer = SDL_CreateGPUBuffer(device, &indexInfo);
    if (!indexBuffer) { 
        SDL_Log("Failed to create index buffer!"); 
        return false;
    }
    
    SDL_SetGPUBufferName(device, indexBuffer, "Index Buffer");

    return true;
}

bool WorldRenderer::RenderTexture(const Texture* texture) {
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

bool WorldRenderer::InitializePipelines(Window* window, SDL_GPUShader* vertexShader, SDL_GPUShader* fragmentShader) {

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
    targetInfo.has_depth_stencil_target = true;
    targetInfo.depth_stencil_format = GetDepthStencilFormat();

    SDL_GPUDepthStencilState depthStencilState{};
    depthStencilState.enable_depth_test = true;
    depthStencilState.enable_depth_write = true;
    depthStencilState.compare_op = SDL_GPU_COMPAREOP_LESS;

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
    pipelineCreateInfo.depth_stencil_state = depthStencilState;

    if (msaaEnabled) {
        SDL_GPUMultisampleState multisampleState{};
        multisampleState.sample_count = sampleCount;
        pipelineCreateInfo.multisample_state = multisampleState;
    }

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineCreateInfo);

    if (!pipeline) {
        SDL_Log("Failed to create graphics pipeline: %s", SDL_GetError());
        return false;
    }

    SDL_Log("Pipeline created successfully!");
    pipelines[PIPELINE_TYPE::FILL_PIPELINE] = pipeline;

    return true;
}

SDL_GPUTextureFormat WorldRenderer::GetDepthStencilFormat() {
    if (!device) {
        return SDL_GPU_TEXTUREFORMAT_INVALID;
    }

    if (SDL_GPUTextureSupportsFormat(device, 
        SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT,
        SDL_GPU_TEXTURETYPE_2D,
        SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET)) {
        return SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT;
    } else if (SDL_GPUTextureSupportsFormat(device,
        SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
        SDL_GPU_TEXTURETYPE_2D,
        SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET)) {
        return SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT;
    } else {
        SDL_Log("Stencil formats not supported!");
        return SDL_GPU_TEXTUREFORMAT_INVALID;
    }
}

bool WorldRenderer::InitializeSamplers() {
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

    SDL_GPUSamplerCreateInfo samplerInfo{};
    samplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;
    samplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
    samplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    offscreenSampler = SDL_CreateGPUSampler(device, &samplerInfo);

    if (!offscreenSampler) {
        SDL_Log("Failed to create offscreen sampler, %s", SDL_GetError());
        return false;
    }

    return true;
}

bool WorldRenderer::CreateDepthStencil(Window* window) {
    int windowWidth, windowHeight;
    window->GetWindowSize(&windowWidth, &windowHeight);

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_DEPTH_FLOAT, 1.0f);
    SDL_SetNumberProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_STENCIL_NUMBER, 0);

    SDL_GPUTextureCreateInfo depthStencilTextureCreateInfo = {};

    depthStencilTextureCreateInfo.format = GetDepthStencilFormat();
    depthStencilTextureCreateInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    depthStencilTextureCreateInfo.width = static_cast<uint32_t>(windowWidth);
    depthStencilTextureCreateInfo.height = static_cast<uint32_t>(windowHeight);
    depthStencilTextureCreateInfo.layer_count_or_depth = 1;
    depthStencilTextureCreateInfo.num_levels = 1;
    depthStencilTextureCreateInfo.props = props;
    if (msaaEnabled) {
        depthStencilTextureCreateInfo.sample_count = sampleCount;
    }

    depthStencilTexture = SDL_CreateGPUTexture(device, &depthStencilTextureCreateInfo);
    SDL_DestroyProperties(props);

    if (!depthStencilTexture) {
        SDL_Log("Failed to create default depth stencil texture, %s", SDL_GetError());
        return false;
    }

    if (msaaEnabled) {
        SDL_SetGPUTextureName(device, depthStencilTexture, "MSAA Depth Stencil Texture");
    } else {
        SDL_SetGPUTextureName(device, depthStencilTexture, "Depth Stencil Texture");
    }
    return true;
}

bool WorldRenderer::CreateMSAATexture(Window* window) {
    int windowWidth, windowHeight;
    window->GetWindowSize(&windowWidth, &windowHeight);

    SDL_PropertiesID props = SDL_CreateProperties();

    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_R_FLOAT, 0.1f);
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_G_FLOAT, 0.1f);
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_B_FLOAT, 0.1f);
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_A_FLOAT, 1.0f);

    SDL_GPUTextureCreateInfo msaaTextureCreateInfo{
        .format = window->GetGPUSwapchainTextureFormat(),
        .usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET,
        .width = static_cast<uint32_t>(windowWidth),
        .height = static_cast<uint32_t>(windowHeight),
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = sampleCount,
        .props = props,
    };

    msaaTexture = SDL_CreateGPUTexture(device, &msaaTextureCreateInfo);
    SDL_DestroyProperties(props);

    if (!msaaTexture) {
        SDL_Log("Failed to create default MSAA texture, %s", SDL_GetError());
        return false;
    }

    SDL_SetGPUTextureName(device, msaaTexture, "MSAA Texture");
    return true;
}

DrawInfo WorldRenderer::UploadVertices(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
    uint32_t verticesSize = vertices.size() * sizeof(Vertex);
    uint32_t indicesSize = indices.size() * sizeof(uint32_t);

    SDL_GPUTransferBufferCreateInfo transferInfo{};
    transferInfo.size = verticesSize + indicesSize;
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;

    auto* transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);
    auto* ptr = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(device, transferBuffer, false));

    std::memcpy(ptr, vertices.data(), verticesSize);
    std::memcpy(ptr + verticesSize, indices.data(), indicesSize);
    SDL_UnmapGPUTransferBuffer(device, transferBuffer);

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);

    SDL_GPUTransferBufferLocation src{ transferBuffer, 0 };
    SDL_GPUBufferRegion dst{ vertexBuffer, vertexBufferOffset, verticesSize };
    SDL_UploadToGPUBuffer(copyPass, &src, &dst, false);

    src.offset = verticesSize;
    dst = { indexBuffer, indexBufferOffset, indicesSize };
    SDL_UploadToGPUBuffer(copyPass, &src, &dst, false);

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_ReleaseGPUTransferBuffer(device, transferBuffer);

    DrawInfo info{
        .indexCount = (uint32_t)indices.size(),
        .firstIndex = indexBufferOffset / sizeof(uint32_t),
        .vertexOffset = (int32_t)(vertexBufferOffset / sizeof(Vertex))
    };

    vertexBufferOffset += verticesSize;
    indexBufferOffset += indicesSize;

    uint32_t maxVertices = 1000;
    uint32_t maxIndices = 3000;

    return info;
}

void WorldRenderer::RegisterSprite(SpriteComponent* sprite) {
    const Texture* texture = sprite->GetTexture();
    if (!texture) return;

    if (!spriteTextures.contains(texture)) {
        if (texture->texture) RenderTexture(texture);

        std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}},
            {{ 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}},
            {{ 0.5f,  0.5f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f}},
        };
        std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };

        spriteDrawInfo[texture] = UploadVertices(vertices, indices);
    }

    spriteTextures[texture].push_back(sprite);
}

void WorldRenderer::DeregisterSprite(SpriteComponent* sprite) {
    const Texture* texture = sprite->GetTexture();
    if (!texture) return;

    auto it = spriteTextures.find(texture);
    if (it == spriteTextures.end()) return;

    auto& vec = it->second;
    vec.erase(std::ranges::find(vec, sprite));

    if (vec.empty()) {
        // SDL_ReleaseGPUTexture(device, texture->texture);
        spriteTextures.erase(it);
    }
}

void WorldRenderer::RegisterMesh(MeshComponent* mesh) {
    const Mesh* m = mesh->GetMesh();
    if (!m) return;

    if (!meshes.contains(m)) {
        if (m->texture) RenderTexture(m->texture);
        meshDrawInfo[m] = UploadVertices(m->vertices, m->indices);
    }
    meshes[m].push_back(mesh);
}

void WorldRenderer::DeregisterMesh(MeshComponent* mesh) {
    const Mesh* m = mesh->GetMesh();
    if (!m) return;

    auto it = meshes.find(m);
    if (it == meshes.end()) return;

    auto& vec = it->second;
    vec.erase(std::ranges::find(vec, mesh));

    if (vec.empty()) {
        // SDL_ReleaseGPUTexture(device, m->texture->texture);
        meshes.erase(it);
    }
}

void WorldRenderer::Shutdown() {
    if (device) {
        SDL_WaitForGPUIdle(device);
    }

    if (offscreenTexture) { SDL_ReleaseGPUTexture(device, offscreenTexture);   offscreenTexture = nullptr; }
    if (msaaTexture) { SDL_ReleaseGPUTexture(device, msaaTexture);        msaaTexture = nullptr; }
    if (depthStencilTexture) { SDL_ReleaseGPUTexture(device, depthStencilTexture); depthStencilTexture = nullptr; }
    if (vertexBuffer) { SDL_ReleaseGPUBuffer(device, vertexBuffer);        vertexBuffer = nullptr; }
    if (indexBuffer) { SDL_ReleaseGPUBuffer(device, indexBuffer);         indexBuffer = nullptr; }
    if (defaultSampler) { SDL_ReleaseGPUSampler(device, defaultSampler);     defaultSampler = nullptr; }
    if (offscreenSampler) { SDL_ReleaseGPUSampler(device, offscreenSampler);   offscreenSampler = nullptr; }

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