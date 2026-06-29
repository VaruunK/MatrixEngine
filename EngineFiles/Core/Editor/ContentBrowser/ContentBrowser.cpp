#include "ContentBrowser.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"
#include <imgui_impl_sdlgpu3.h>

ContentBrowser::ContentBrowser() : controller(this) {
    controller.Start();
}

void ContentBrowser::Render(bool* active) {
    ImGuiWindowFlags contentBarFlags = ImGuiWindowFlags_NoCollapse;
    if (locked) {
        contentBarFlags = contentBarFlags | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    }
    if (ImGui::Begin("Content Browser", active, contentBarFlags)) {

        bool isFocused = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
        if (isFocused && !wasFocused) {
            GEventBUS.Notify(EVENT_CONTENT_BROWSER_HOVERED);
        }
        wasFocused = isFocused;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PopStyleVar();

        if (lockPopup) {
            ImGui::OpenPopup("my_select_popup");
        }
        RenderLockPopup();

        if (ImGui::TreeNode("Content")) {            
            RenderContentFolder("Content");
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ContentBrowser::RenderLockPopup() {
    if (ImGui::BeginPopup("my_select_popup")) {
        const char* names[] = { "Lock", "Unlock" };

        for (int i = 0; i < IM_COUNTOF(names); i++) {
            bool isDisabled = (i == 0) ? locked : !locked;
            ImGuiSelectableFlags flags = isDisabled ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None;

            if (ImGui::Selectable(names[i], false, flags)) {
                locked = (i == 0);
                unlocked = !locked;
                lockPopup = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }
}

void ContentBrowser::RenderContentFolder(const std::filesystem::path& path) {
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_directory()) {
            if (ImGui::TreeNode(entry.path().filename().string().c_str())) {
                RenderContentFolder(entry.path());
                ImGui::TreePop();
            }
        } else {
            ImGui::BulletText("%s", entry.path().filename().string().c_str());
        }
    }
}

void ContentBrowser::Tick(float deltaTime) {
    controller.Tick(deltaTime);
}

void ContentBrowser::OpenLockPopup() {
    lockPopup = true;
}

void ContentBrowser::CloseLockPopup() {
    lockPopup = false;
}
