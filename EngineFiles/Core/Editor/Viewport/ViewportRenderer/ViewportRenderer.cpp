#include "ViewportRenderer.hpp"
#include "Core/Editor/Viewport/Viewport.hpp"
#include "Core/ShaderManager/ShaderManager.hpp"
#include "Core/GameObject/World/WorldRenderer/WorldRenderer.hpp"
#include "Core/Structs/Appstate.hpp"
#include "Core/Structs/FrameData.hpp"
#include "Core/GameObject/Component/MeshComponent/MeshComponent.hpp"
#include "Core/GameObject/Component/SpriteComponent/SpriteComponent.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"
#include <SDL3/SDL_video.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include <glm/glm.hpp>

ViewportRenderer::ViewportRenderer(Appstate& appstate, Viewport* viewport, WorldRenderer& worldRenderer) 
    : appstate(appstate), worldRenderer(worldRenderer) {
    this->viewport = viewport;

    io = &ImGui::GetIO();

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

    SDL_GPUShader* vert = ShaderManager::Get().LoadShader(vertShader, &optionsVert);
    SDL_GPUShader* frag = ShaderManager::Get().LoadShader(fragShader, &optionsFrag);

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

    if (!InitializeSamplers()) {
        SDL_Log("Failed to create Samplers");
        return;
    }

    GEventBUS.Subscribe(SDL_EVENT_WINDOW_RESIZED, [this]() {
        SDL_WaitForGPUIdle(this->appstate.device);

        SDL_ReleaseGPUTexture(this->appstate.device, selectProxyTexture);
        selectProxyTexture = nullptr;

        SDL_ReleaseGPUTexture(this->appstate.device, selectProxyDepthTexture);
        selectProxyDepthTexture = nullptr;

        CreateSelectProxyTexture();
        CreateSelectProxyDepthTexture(); }
    );
}

ViewportRenderer::~ViewportRenderer() {
    if (appstate.device) {
        SDL_WaitForGPUIdle(appstate.device);
    }
    if (selectProxyTexture) {
        SDL_ReleaseGPUTexture(appstate.device, selectProxyTexture);
        selectProxyTexture = nullptr;
    }
    if (defaultSampler) {
        SDL_ReleaseGPUSampler(appstate.device, defaultSampler);
        defaultSampler = nullptr;
    }
    if (selectProxyDepthTexture) {
        SDL_ReleaseGPUTexture(appstate.device, selectProxyDepthTexture);
        selectProxyDepthTexture = nullptr;
    }
    if (selectProxyPipeline) {
        SDL_ReleaseGPUGraphicsPipeline(appstate.device, selectProxyPipeline);
        selectProxyPipeline = nullptr;
    }

    ResetIds();
}

void ViewportRenderer::Render(FrameData& frame) {

    worldRenderer.Render(frame);

    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    if (ImGui::BeginTabItem("Viewport", nullptr,
        ImGuiTabItemFlags_NoCloseWithMiddleMouseButton | ImGuiTabItemFlags_NoReorder)) {
        
        bool isFocused = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
        if (isFocused && !wasFocused) {
            GEventBUS.Notify(EVENT_VIEWPORT_HOVERED);
        }
        wasFocused = isFocused;

        ImGui::Checkbox("Show FPS", &showFPS);
        
        ImGui::SameLine();

        float w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.y) * 0.3f;
        ImGui::SetNextItemWidth(w);
        static ImGuiSliderFlags sliderFlags = ImGuiSliderFlags_ClampOnInput;
        if (ImGui::SliderInt("Camera Speed", &speed, 1, 10, "%d", sliderFlags)) {
            // speed = std::max(speed, 1);
            viewport->SetCameraSpeed(speed);
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2 / 7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2 / 7.0f, 0.8f, 0.8f));

        static int clicked = 0;
        
        if (ImGui::Button("Play"))
            clicked++;
        if (clicked & 1) {

        }
        ImGui::PopStyleColor(3);

        ImVec2 imagePos = ImGui::GetCursorScreenPos();
        ImVec2 imageSize = ImGui::GetContentRegionAvail();
        if (imageSize.x > 0 && imageSize.y > 0 && frame.viewportTexture) {
            ImVec2 imagePos = ImGui::GetCursorScreenPos();

            ImTextureRef texRef = (ImTextureID)(intptr_t)frame.viewportTexture;
            ImGui::Image(texRef, imageSize);

            if (ImGui::IsItemClicked()) {
                ImVec2 mouse = ImGui::GetMousePos();

                float localX = mouse.x - imagePos.x;
                float localY = mouse.y - imagePos.y;

                // Scale from display size to texture size
                int texW, texH;
                SDL_GetWindowSize(appstate.window, &texW, &texH);
                // GEventBUS.Notify(SDL_EVENT_USER);
                viewport->SetClickedPosition(static_cast<int>(localX * (texW / imageSize.x)), 
                    static_cast<int>(localY * (texH / imageSize.y)));

                GEventBUS.Notify(EVENT_VIEWPORT_CLICKED);
            }

            if (showFPS) {
                char fpsText[16];
                snprintf(fpsText, sizeof(fpsText), "FPS: %.2f", 1.0f / viewport->deltaSeconds);

                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImVec2 textPos = ImVec2(imagePos.x + 8.0f, imagePos.y + 8.0f);
                drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 200), fpsText);
                drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), fpsText);
            }
        }
        ImGui::EndTabItem();
    }
}

bool ViewportRenderer::InitializeSamplers() {
    SDL_GPUSamplerCreateInfo samplerCreateInfo = {
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
    };

    defaultSampler = SDL_CreateGPUSampler(appstate.device, &samplerCreateInfo);

    if (!defaultSampler) {
        SDL_Log("Failed to create default sampler, %s", SDL_GetError());
        return false;
    }
    return true;
}

void ViewportRenderer::RenderSelectProxy(SDL_GPUCommandBuffer* commandBuffer) {
    ResetIds();

    const View& view = viewport->GetCameraView();

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

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(commandBuffer, &colorTarget, 1, &depthInfo);
    SDL_BindGPUGraphicsPipeline(pass, selectProxyPipeline);

    SDL_GPUBufferBinding vb{ worldRenderer.vertexBuffer, 0 };
    SDL_BindGPUVertexBuffers(pass, 0, &vb, 1);

    SDL_GPUBufferBinding ib{ worldRenderer.indexBuffer, 0 };
    SDL_BindGPUIndexBuffer(pass, &ib, SDL_GPU_INDEXELEMENTSIZE_32BIT);

    int w, h;
    SDL_GetWindowSize(appstate.window, &w, &h);
    float aspect = static_cast<float>(w) / static_cast<float>(h);

    for (auto& [mesh, components] : worldRenderer.meshes) {
        const DrawInfo& di = worldRenderer.meshDrawInfo.at(mesh);

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

    for (auto& [texture, sprites] : worldRenderer.spriteTextures) {
        const DrawInfo& di = worldRenderer.spriteDrawInfo.at(texture);

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

Entity* ViewportRenderer::ReadPixel(SDL_GPUCommandBuffer* commandBuffer, int x, int y) {
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
    uint8_t* ptr = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(appstate.device, downloadBuffer, false));

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

bool ViewportRenderer::InitializeSelectProxyPipeline(SDL_GPUShader* vertexShader, SDL_GPUShader* fragmentShader) {
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

    if (!selectProxyPipeline) {
        SDL_Log("Pipeline result: %p, error: %s", selectProxyPipeline, SDL_GetError());
    }

    return selectProxyPipeline != nullptr;
}

bool ViewportRenderer::CreateSelectProxyDepthTexture() {
    int w, h;
    SDL_GetWindowSize(appstate.window, &w, &h);

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_DEPTH_FLOAT, 1.0f);
    SDL_SetNumberProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_STENCIL_NUMBER, 0);

    SDL_GPUTextureCreateInfo info{};
    info.format = GetDepthStencilFormat();
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

bool ViewportRenderer::CreateSelectProxyTexture() {
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

SDL_GPUTextureFormat ViewportRenderer::GetDepthStencilFormat() {
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