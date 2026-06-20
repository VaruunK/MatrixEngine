#include "ContentBrowser.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"

ContentBrowser::ContentBrowser() {
    contentBarFlags = ImGuiWindowFlags_NoCollapse;

    GEventBUS.Subscribe(EVENT_FOCUS_CHANGED, [this]() {
        
        }
    );
}

void ContentBrowser::Render(bool* active) {
    if (ImGui::Begin("Content Browser", active, contentBarFlags)) {
        
        bool isFocused = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
        if (isFocused && !wasFocused) {
            GEventBUS.Notify(EVENT_CONTENT_BROWSER_HOVERED);
        }
        wasFocused = isFocused;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PopStyleVar();
        if (ImGui::TreeNode("Content")) {

            RenderContentFolder("Content");
            ImGui::TreePop();
        }

        if (ImGui::IsItemHovered()) {
            GEventBUS.Notify(EVENT_CONTENT_BROWSER_HOVERED);
        }

    }
    ImGui::End();
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