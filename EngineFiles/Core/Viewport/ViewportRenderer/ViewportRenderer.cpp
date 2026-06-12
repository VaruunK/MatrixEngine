#include "ViewportRenderer.hpp"
#include "Core/Viewport/Viewport.hpp"
#include "Core/Structs/FrameData.hpp"
#include <SDL3/SDL_video.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include <glm/glm.hpp>

std::unique_ptr<ShaderManager> ViewportRenderer::shaderManager = nullptr;

ViewportRenderer::ViewportRenderer(SDL_GPUDevice* device, SDL_Window* window, Viewport* viewport) {
	this->device = device;
    this->window = window;
    this->viewport = viewport;
}

bool ViewportRenderer::Initialize() {
    ShaderManager* rawSM = new ShaderManager(device);
    if (!rawSM) {
        SDL_Log("Failed to create Shader Manager");
        return false;
    }

    shaderManager.reset(rawSM);

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
    init_info.Device = device;
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(device, window);
    init_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
    init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;
    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;

    if (!ImGui_ImplSDLGPU3_Init(&init_info)) {
        SDL_Log("Failed to initialize ImGui SDLGPU3 backend");
        return false;
    }

    if (!ImGui_ImplSDL3_InitForSDLGPU(window)) {
        SDL_Log("Failed to initialize ImGui SDL3 backend");
        return false;
    }

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

    if (!frame.commandBuffer || !frame.swapchainTexture) return;
    
    ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    glm::vec4 clear_color = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImTextureRef texRef = (ImTextureID)(intptr_t)frame.viewportTexture;

    ImGui::SetNextWindowSize(ImVec2(io->DisplaySize.x, io->DisplaySize.y));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGuiWindowFlags engineFlags = ImGuiWindowFlags_MenuBar | 
        ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("Engine", nullptr, engineFlags);

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
    }

    ImGuiWindowFlags viewportFlags = ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoResize; // | ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr, viewportFlags);
    ImGui::PopStyleVar();

    ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton);

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

        ImVec2 size = ImGui::GetContentRegionAvail();
        if (size.x > 0 && size.y > 0 && frame.viewportTexture) {
            ImVec2 imagePos = ImGui::GetCursorScreenPos();

            ImTextureRef texRef = (ImTextureID)(intptr_t)frame.viewportTexture;
            ImGui::Image(texRef, size);

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

    ImGui::EndTabBar();
    
    ImGui::End();
    
    ImGui::End();
    // Rendering
    ImGui::Render();

    ImDrawData* draw_data = ImGui::GetDrawData();
    const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
    if (is_minimized) return;

    // ImGui pass on top of whatever WorldRenderer already drew
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


SDL_GPUTextureFormat ViewportRenderer::GetDepthStencilFormat() {
    if (!device) {
        return SDL_GPU_TEXTUREFORMAT_INVALID;
    }

    if (SDL_GPUTextureSupportsFormat(device,
        SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT,
        SDL_GPU_TEXTURETYPE_2D,
        SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET)) {
        return SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT;
    }
    else if (SDL_GPUTextureSupportsFormat(device,
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

    defaultSampler = SDL_CreateGPUSampler(device, &samplerCreateInfo);

    if (!defaultSampler) {
        SDL_Log("Failed to create default sampler, %s", SDL_GetError());
        return false;
    }
    return true;
}


void ViewportRenderer::Shutdown() {
    SDL_WaitForGPUIdle(device);
    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui::DestroyContext();
}