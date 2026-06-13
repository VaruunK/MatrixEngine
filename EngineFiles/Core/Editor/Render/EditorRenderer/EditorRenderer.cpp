#include "EditorRenderer.hpp"
#include "Core/Structs/Appstate.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

EditorRenderer::EditorRenderer(Appstate& appstate, WorldRenderer* worldRenderer) 
	: viewport(appstate, worldRenderer), appstate(appstate) {

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

	viewport.Initialize();
}

void EditorRenderer::Tick(float deltaTime) {
	viewport.Tick(deltaTime);
}

uint32_t EditorRenderer::AssignId(Entity* entity)
{
    return 0;
}

uint32_t EditorRenderer::ReadPixel(SDL_GPUCommandBuffer* commandBuffer, int x, int y)
{
    return 0;
}

void EditorRenderer::Render() {
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

            ImGuiWindowFlags viewportFlags = ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoResize; // | ImGuiWindowFlags_NoMove;

            ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::Begin("Viewport", nullptr, viewportFlags);
            ImGui::PopStyleVar();

            ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton);

            viewport.Render(frame);
        }

        ImGui::EndTabBar();

        ImGui::End();

        ImGui::End();

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