#include "EditorRenderer.hpp"
#include "Core/Structs/EditorInfo.hpp"
#include "Core/Editor/Viewport/Viewport.hpp"
#include "Core/Editor/ContentBrowser/ContentBrowser.hpp"
#include "Core/Editor/GameObjectDetailsPanel/GameObjectDetailsPanel.hpp"
#include "Core/Structs/Appstate.hpp"
#include "Core/Structs/FrameData.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include <iostream>
#include <SDL3/SDL_log.h>

EditorRenderer::EditorRenderer(Appstate& appstate, EditorInfo& info, WorldRenderer& worldRenderer) 
    : appstate(appstate), info(info) {

    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    SDL_SetGPUTextureName(appstate.device, swapchainTexture, "Swapcahin Texture");
}

void EditorRenderer::Render() {

    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(appstate.device);
    if (!commandBuffer) return;

    swapchainTexture = nullptr;
    SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, appstate.window, &swapchainTexture, nullptr, nullptr);

    FrameData frame{};
    frame.commandBuffer = commandBuffer;
    frame.swapchainTexture = swapchainTexture;

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
                
                RenderMenuBar();

                ImGui::EndMainMenuBar();

                ImGui::SetNextWindowSize(ImVec2(300, 720), ImGuiCond_FirstUseEver);

                if (contentBrowserViewOption && !contentBrowserOpen) {
                    contentBrowserOpen = true;
                    contentBrowserViewOption = false;
                }

                if (contentBrowserOpen) {   
                    info.contentBrowser.Render(&contentBrowserOpen);
                }

                if (detailsPanelViewOption && !detailsPanelOpen) {
                    detailsPanelOpen = true;
                    detailsPanelViewOption = false;
                }

                if (detailsPanelOpen) {
                    info.detailsPanel.Render(&detailsPanelOpen);
                }
                
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
                    info.viewport.Render(frame);

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

void EditorRenderer::RenderMenuBar() {
    if (ImGui::BeginMenu("File")) {
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
        if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {}
        ImGui::Separator();
        if (ImGui::MenuItem("Cut", "Ctrl+X")) {}
        if (ImGui::MenuItem("Copy", "Ctrl+C")) {}
        if (ImGui::MenuItem("Paste", "Ctrl+V")) {}
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem("Content Browser", nullptr, &contentBrowserViewOption)) {}
        if (ImGui::MenuItem("Details Panel", nullptr, &detailsPanelViewOption)) {}
        ImGui::EndMenu();
    }
}

void EditorRenderer::Shutdown() {
    if (swapchainTexture) {
        SDL_ReleaseGPUTexture(appstate.device, swapchainTexture);
    }
}