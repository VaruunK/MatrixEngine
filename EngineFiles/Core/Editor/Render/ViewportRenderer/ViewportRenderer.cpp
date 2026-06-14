#include "ViewportRenderer.hpp"
#include "Core/Editor/Viewport/Viewport.hpp"
#include "Core/Structs/Appstate.hpp"
#include "Core/Structs/FrameData.hpp"
#include <SDL3/SDL_video.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include <glm/glm.hpp>

std::unique_ptr<ShaderManager> ViewportRenderer::shaderManager = nullptr;

ViewportRenderer::ViewportRenderer(Appstate& appstate, Viewport* viewport) : appstate(appstate) {
    this->viewport = viewport;
}

bool ViewportRenderer::Initialize() {
    ShaderManager* rawSM = new ShaderManager(appstate.device);
    if (!rawSM) {
        SDL_Log("Failed to create Shader Manager");
        return false;
    }

    io = &ImGui::GetIO();

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

    if (!InitializeSamplers()) {
        SDL_Log("Failed to create Samplers");
        return false;
    }

    return true;
}

void ViewportRenderer::Render(FrameData& frame) {

    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);
    active = false;

    if (bool tabActive = ImGui::BeginTabItem("Viewport", nullptr,
        ImGuiTabItemFlags_NoCloseWithMiddleMouseButton | ImGuiTabItemFlags_NoReorder)) {

        active = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

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

                viewport->mouseClickX = static_cast<int>(localX * (texW / imageSize.x));
                viewport->mouseClickY = static_cast<int>(localY * (texH / imageSize.y));
                viewport->clicked = true;
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


void ViewportRenderer::Shutdown() {
    SDL_WaitForGPUIdle(appstate.device);
    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui::DestroyContext();
}