#include "EditorRenderer.hpp"
#include "Core/Structs/Appstate.hpp"
#include "Core/Editor/Render/WorldRenderer/WorldRenderer.hpp"
#include "Core/GameObject/Component/MeshComponent/MeshComponent.hpp"
#include "Core/GameObject/Component/SpriteComponent/SpriteComponent.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include <iostream>

std::unique_ptr<ShaderManager> EditorRenderer::shaderManager = nullptr;

EditorRenderer::EditorRenderer(Appstate& appstate, WorldRenderer* worldRenderer) 
	: viewport(appstate, worldRenderer), appstate(appstate) {

    this->worldRenderer = worldRenderer;

    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplSDLGPU3_InitInfo init_info = {};
    init_info.Device = appstate.device;
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(appstate.device, appstate.window);
    init_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
    init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;
    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;

    if (!ImGui_ImplSDLGPU3_Init(&init_info)) {
        SDL_Log("Failed to initialize ImGui SDLGPU3 backend");
        return;
    }

    if (!ImGui_ImplSDL3_InitForSDLGPU(appstate.window)) {
        SDL_Log("Failed to initialize ImGui SDL3 backend");
        return;
    }

    ShaderManager* rawSM = new ShaderManager(appstate.device);
    if (!rawSM) {
        SDL_Log("Failed to create Shader Manager");
        return;
    }
    shaderManager.reset(rawSM);

    ShaderOptions optionsVert = {
        .num_samplers = 0,
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 1
    };

    ShaderOptions optionsFrag = {
        .num_samplers = 0,
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 1
    };

    std::string vertShader = "shaders/SelectProxy.vert.hlsl";
    std::string fragShader = "shaders/SelectProxy.frag.hlsl";

    SDL_GPUShader* vert = shaderManager.get()->LoadShader(vertShader, &optionsVert);
    SDL_GPUShader* frag = shaderManager.get()->LoadShader(fragShader, &optionsFrag);

    if (!CreateSelectProxyDepthTexture()) {
        SDL_Log("Failed to initialize Select Proxy Depth Texture");
        return;
    }

    if (!CreateSelectProxyTexture()) {
        SDL_Log("Failed to create Select Proxy Texture");
        return;
    }

    if (!InitializeSelectProxyPipeline(vert, frag)) {
        SDL_Log("Failed to initialize Select Proxy Pipeline");
        return;
    }

	viewport.Initialize();
}

EditorRenderer::~EditorRenderer() {
    if (appstate.device) {
        SDL_WaitForGPUIdle(appstate.device);
    }
    if (selectProxyTexture) {
        SDL_ReleaseGPUTexture(appstate.device, selectProxyTexture);
        selectProxyTexture = nullptr;
    }
    if (selectProxyDepthTexture) {
        SDL_ReleaseGPUTexture(appstate.device, selectProxyDepthTexture);
        selectProxyDepthTexture = nullptr;
    }
    SDL_ReleaseGPUGraphicsPipeline(appstate.device, selectProxyPipeline);
    
    if (shaderManager) {
        shaderManager->Shutdown();
        shaderManager.reset();
    }
}

void EditorRenderer::Tick(float deltaTime) {
	viewport.Tick(deltaTime);
}

void EditorRenderer::RenderSelectProxy(SDL_GPUCommandBuffer* commandBuffer) {
    ResetIds();

    const View& view = viewport.GetCameraView();

    SDL_GPUColorTargetInfo colorTarget{};
    colorTarget.clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
    colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTarget.texture = selectProxyTexture;
    colorTarget.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPUDepthStencilTargetInfo depthInfo{};
    depthInfo.clear_depth = 1.0f;
    depthInfo.clear_stencil = 0;
    depthInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    depthInfo.store_op = SDL_GPU_STOREOP_STORE;
    depthInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;
    depthInfo.texture = selectProxyDepthTexture;

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(
        commandBuffer, &colorTarget, 1, &depthInfo
    );
    SDL_BindGPUGraphicsPipeline(pass, selectProxyPipeline);

    SDL_GPUBufferBinding vb{ worldRenderer->vertexBuffer, 0 };
    SDL_BindGPUVertexBuffers(pass, 0, &vb, 1);

    SDL_GPUBufferBinding ib{ worldRenderer->indexBuffer, 0 };
    SDL_BindGPUIndexBuffer(pass, &ib, SDL_GPU_INDEXELEMENTSIZE_32BIT);

    int w, h;
    SDL_GetWindowSize(appstate.window, &w, &h);
    float aspect = static_cast<float>(w) / static_cast<float>(h);

    for (auto& [mesh, components] : worldRenderer->meshes) {
        const DrawInfo& di = worldRenderer->meshDrawInfo.at(mesh);

        for (auto* component : components) {
            uint32_t id = nextId++;
            idToMesh[id] = component;

            glm::mat4 mvp = view.projectionMatrix * view.viewMatrix * component->GetModelMatrix(aspect);
            glm::vec4 idColor = EncodeId(id);

            SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvp, sizeof(mvp));
            SDL_PushGPUFragmentUniformData(commandBuffer, 0, &idColor, sizeof(idColor));
            SDL_DrawGPUIndexedPrimitives(
                pass, di.indexCount, 1, di.firstIndex, di.vertexOffset, 0
            );
        }
    }

    for (auto& [texture, sprites] : worldRenderer->spriteTextures) {
        const DrawInfo& di = worldRenderer->spriteDrawInfo.at(texture);

        for (auto* sprite : sprites) {
            uint32_t id = nextId++;
            idToSprite[id] = sprite;

            glm::mat4 mvp = view.projectionMatrix
                * view.viewMatrix
                * sprite->GetModelMatrix(aspect);
            glm::vec4 idColor = EncodeId(id);

            SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvp, sizeof(mvp));
            SDL_PushGPUFragmentUniformData(commandBuffer, 0, &idColor, sizeof(idColor));
            SDL_DrawGPUIndexedPrimitives(
                pass, di.indexCount, 1, di.firstIndex, di.vertexOffset, 0
            );
        }
    }

    SDL_EndGPURenderPass(pass);
}

Entity* EditorRenderer::ReadPixel(SDL_GPUCommandBuffer* commandBuffer, int x, int y) {
    SDL_GPUTransferBufferCreateInfo transferInfo{};
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    transferInfo.size = 4;

    SDL_GPUTransferBuffer* downloadBuffer = SDL_CreateGPUTransferBuffer(appstate.device, &transferInfo);

    if (!downloadBuffer) {
        SDL_SubmitGPUCommandBuffer(commandBuffer);
        return nullptr;
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

    SDL_GPUTextureRegion region{};
    region.texture = selectProxyTexture;
    region.x = static_cast<uint32_t>(x);
    region.y = static_cast<uint32_t>(y);
    region.w = 1;
    region.h = 1;
    region.d = 1;

    SDL_GPUTextureTransferInfo dst{};
    dst.transfer_buffer = downloadBuffer;
    dst.offset = 0;
    dst.pixels_per_row = 1;
    dst.rows_per_layer = 1;

    SDL_DownloadFromGPUTexture(copyPass, &region, &dst);
    SDL_EndGPUCopyPass(copyPass);

    // Submit and stall
    SDL_SubmitGPUCommandBuffer(commandBuffer);
    SDL_WaitForGPUIdle(appstate.device);

    // Read the pixel bytes
    uint8_t* ptr = static_cast<uint8_t*>(
        SDL_MapGPUTransferBuffer(appstate.device, downloadBuffer, false)
        );

    Entity* result = nullptr;

    if (ptr) {
        uint32_t id = DecodeId(ptr[0], ptr[1], ptr[2]);
        SDL_UnmapGPUTransferBuffer(appstate.device, downloadBuffer);

        if (id != 0) {
            auto meshIt = idToMesh.find(id);
            if (meshIt != idToMesh.end()) {
                result = meshIt->second->GetOwner();
            }
            else {
                auto spriteIt = idToSprite.find(id);
                if (spriteIt != idToSprite.end()) {
                    result = spriteIt->second->GetOwner();
                }
            }
        }
    }

    SDL_ReleaseGPUTransferBuffer(appstate.device, downloadBuffer);
    return result;
}

void EditorRenderer::Render() {
    if (resized) {
        SDL_WaitForGPUIdle(appstate.device);

        SDL_ReleaseGPUTexture(appstate.device, selectProxyTexture);
        SDL_ReleaseGPUTexture(appstate.device, selectProxyDepthTexture);

        CreateSelectProxyTexture();
        CreateSelectProxyDepthTexture();

        resized = false;
    }

    if (viewport.clicked) {
        viewport.clicked = false;

        SDL_GPUCommandBuffer* pickCmd = SDL_AcquireGPUCommandBuffer(appstate.device);
        if (pickCmd) {
            RenderSelectProxy(pickCmd);
            Entity* picked = ReadPixel(pickCmd, viewport.mouseClickX, viewport.mouseClickY);
            /*if (picked) {
                std::cout << picked << std::endl;
            }*/
        }
    }

    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(appstate.device);
    if (!commandBuffer) return;

    SDL_GPUTexture* swapchain = nullptr;
    SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, appstate.window, &swapchain, nullptr, nullptr);

    FrameData frame{};
    frame.commandBuffer = commandBuffer;
    frame.swapchainTexture = swapchain;

    if (frame.commandBuffer && frame.swapchainTexture) {
        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        glm::vec4 clear_color = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);

        ImTextureRef texRef = (ImTextureID)(intptr_t)frame.viewportTexture;

        ImGui::SetNextWindowSize(ImVec2(io->DisplaySize.x, io->DisplaySize.y));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGuiWindowFlags engineFlags = ImGuiWindowFlags_MenuBar |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        if (ImGui::Begin("Engine", nullptr, engineFlags)) {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit")) {
                    if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
                    if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {} // Disabled item
                    ImGui::Separator();
                    if (ImGui::MenuItem("Cut", "Ctrl+X")) {}
                    if (ImGui::MenuItem("Copy", "Ctrl+C")) {}
                    if (ImGui::MenuItem("Paste", "Ctrl+V")) {}
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();

                ImGuiWindowFlags viewportFlags = ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoTitleBar |
                    ImGuiWindowFlags_NoBackground |
                    ImGuiWindowFlags_NoResize; // | ImGuiWindowFlags_NoMove;

                ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
                
                if (ImGui::Begin("Viewport", nullptr, viewportFlags)) {

                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                    
                    ImGui::PopStyleVar();

                    ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton);

                    // Render viewport tab
                    viewport.Render(frame);

                    ImGui::EndTabBar();
                }
                // End Viewport
                ImGui::End();  
            }
            // End Engine
            ImGui::End();
        }

        ImGui::Render();

        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (is_minimized) return;

        SDL_GPUColorTargetInfo imguiTarget{};
        imguiTarget.texture = frame.swapchainTexture;
        imguiTarget.clear_color = { 0.05f, 0.05f, 0.05f, 1.0f };
        imguiTarget.load_op = SDL_GPU_LOADOP_CLEAR;
        imguiTarget.store_op = SDL_GPU_STOREOP_STORE;

        ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, frame.commandBuffer);
        SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(frame.commandBuffer, &imguiTarget, 1, nullptr);
        ImGui_ImplSDLGPU3_RenderDrawData(draw_data, frame.commandBuffer, pass);
        SDL_EndGPURenderPass(pass);
    }

    if (!SDL_SubmitGPUCommandBuffer(frame.commandBuffer)) {
        SDL_Log("Failed to submit: %s", SDL_GetError());
    }
}

bool EditorRenderer::InitializeSelectProxyPipeline(SDL_GPUShader* vertexShader, SDL_GPUShader* fragmentShader) {
    SDL_GPUColorTargetDescription colorTargetDescription{};

    colorTargetDescription.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    colorTargetDescription.blend_state.enable_blend = false;

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

    SDL_GPUVertexAttribute positionAttr{};
    positionAttr.location = 0;
    positionAttr.buffer_slot = 0;
    positionAttr.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    positionAttr.offset = offsetof(Vertex, position);

    SDL_GPUVertexBufferDescription vertexBufferDesc{};
    vertexBufferDesc.slot = 0;
    vertexBufferDesc.pitch = sizeof(Vertex);
    vertexBufferDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertexBufferDesc.instance_step_rate = 0;

    SDL_GPUVertexInputState vertexInputState{};
    vertexInputState.vertex_attributes = &positionAttr;
    vertexInputState.num_vertex_attributes = 1;
    vertexInputState.vertex_buffer_descriptions = &vertexBufferDesc;
    vertexInputState.num_vertex_buffers = 1;

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.vertex_shader = vertexShader;
    pipelineCreateInfo.fragment_shader = fragmentShader;
    pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    pipelineCreateInfo.target_info = targetInfo;
    pipelineCreateInfo.vertex_input_state = vertexInputState;
    pipelineCreateInfo.depth_stencil_state = depthStencilState;

    selectProxyPipeline = SDL_CreateGPUGraphicsPipeline(appstate.device, &pipelineCreateInfo);
    SDL_Log("Pipeline result: %p, error: %s", selectProxyPipeline, SDL_GetError());
    return selectProxyPipeline != nullptr;
}

bool EditorRenderer::CreateSelectProxyTexture() {
    int w, h;
    SDL_GetWindowSize(appstate.window, &w, &h);

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_R_FLOAT, 0.0f);
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_G_FLOAT, 0.0f);
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_B_FLOAT, 0.0f);
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_A_FLOAT, 1.0f);

    SDL_GPUTextureCreateInfo info{};
    info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    info.width = static_cast<uint32_t>(w);
    info.height = static_cast<uint32_t>(h);
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    info.props = props;

    selectProxyTexture = SDL_CreateGPUTexture(appstate.device, &info);
    SDL_DestroyProperties(props);

    if (selectProxyTexture) {
        SDL_SetGPUTextureName(appstate.device, selectProxyTexture, "Select Proxy Texture");
    }

    return selectProxyTexture != nullptr;
}

bool EditorRenderer::CreateSelectProxyDepthTexture() {
    int w, h;
    SDL_GetWindowSize(appstate.window, &w, &h);

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_DEPTH_FLOAT, 1.0f);
    SDL_SetNumberProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_STENCIL_NUMBER, 0);

    SDL_GPUTextureCreateInfo info{};
    info.format = GetDepthStencilFormat();
    SDL_Log("Creating select proxy depth texture with format: %d", (int)info.format);
    info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    info.width = static_cast<uint32_t>(w);
    info.height = static_cast<uint32_t>(h);
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    info.props = props;

    selectProxyDepthTexture = SDL_CreateGPUTexture(appstate.device, &info);
    SDL_DestroyProperties(props);

    if (selectProxyTexture) {
        SDL_SetGPUTextureName(appstate.device, selectProxyTexture, "Select Proxy Depth Stencil Texture");
    }

    return selectProxyDepthTexture != nullptr;
}

SDL_GPUTextureFormat EditorRenderer::GetDepthStencilFormat() {
    if (!appstate.device) {
        return SDL_GPU_TEXTUREFORMAT_INVALID;
    }
    
    if (SDL_GPUTextureSupportsFormat(appstate.device,
        SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT,
        SDL_GPU_TEXTURETYPE_2D,
        SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET)) {
        return SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT;
    }
    else if (SDL_GPUTextureSupportsFormat(appstate.device,
        SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
        SDL_GPU_TEXTURETYPE_2D,
        SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET)) {
        return SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT;
    }
    else {
        SDL_Log("Stencil formats not supported!");
        return SDL_GPU_TEXTUREFORMAT_INVALID;
    }
}