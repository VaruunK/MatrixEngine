#include "GameObjectDetailsPanel.hpp"
#include "Core/GameObject/GameObject.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"
#include <imgui_impl_sdlgpu3.h>
#include <array>
#include <iostream>
#include <Core/Structs/Transform.hpp>

GameObjectDetailsPanel::GameObjectDetailsPanel() {
    controller.BindMouseButton(SDL_BUTTON_LEFT,
        [this]() { popup = false; }
    );

    controller.BindMouseButton(SDL_BUTTON_RIGHT,
        [this]() { popup = true; }
    );

    controller.Start();
}

void GameObjectDetailsPanel::SetGameObjectToView(GameObject* gameObject) {
	detailGameObject = gameObject;
}

void GameObjectDetailsPanel::Render(bool* active) {
    ImGuiWindowFlags contentBarFlags = ImGuiWindowFlags_NoCollapse;
    if (locked) {
        contentBarFlags = contentBarFlags | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    }

    if (ImGui::Begin("Details", active, contentBarFlags)) {

        bool isFocused = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
        if (isFocused && !wasFocused) {
            GEventBUS.Notify(EVENT_DETAILS_PANEL_HOVERED);
        }
        wasFocused = isFocused;

        RenderRightClickPopup();

        if (popup) {
            ImGui::OpenPopup("options_popup");
            popup = false;
        }

        if (detailGameObject) {
            const ReflectedClass& rc = detailGameObject->GetClass();
            ImGui::Text("Class: %s", rc.name.c_str());
            ImGui::Text("Parent: %s", rc.parent.c_str());
            ImGui::Separator();

            std::array ar = { "public", "protected", "private" };
            for (auto& access : ar) {
                if (rc.fields.contains(access)) {
                    if (ImGui::TreeNode(access)) {
                        for (auto& field : rc.fields.at(access)) {
                            ImGui::Separator();
                            ImGui::Text(field.typeName.c_str());
                            ImGui::SameLine();
                            if (field.typeName == "Transform") {
                                if (ImGui::TreeNode(field.name.c_str())) {
                                    Transform t;
                                    detailGameObject->GetFieldValue(field, detailGameObject, &t);
                                    const ReflectedStruct& info = Transform::StaticStruct();
                                    for (auto& member : info.members) {
                                        glm::vec3 vector;
                                        const std::string& memberName = member.first;
                                        const std::string& memberType = member.second;
                                        GetStructMemberValue(info, memberName, &t, &vector);
                                        ImGui::Text(memberName.c_str());
                                        ImGui::Text("X: %.2f Y: %.2f, Z: %.2f", vector.x, vector.y, vector.z);
                                        ImGui::Separator();
                                    }
                                    ImGui::TreePop();
                                }
                            }
                        }
                        ImGui::TreePop();
                    }
                }
            }

            
        } else {
            ImGui::Text("Class: None");
        }
    }
    ImGui::End();
}

void GameObjectDetailsPanel::RenderRightClickPopup() {
    if (ImGui::BeginPopup("options_popup")) {

        // ImGui::Separator();
        const char* names[] = { "Lock", "Unlock" };

        for (int i = 0; i < IM_COUNTOF(names); i++) {
            bool isDisabled = (i == 0) ? locked : !locked;
            ImGuiSelectableFlags flags = isDisabled ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None;

            if (ImGui::Selectable(names[i], false, flags)) {
                locked = (i == 0);
                unlocked = !locked;
                popup = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }
}

void GameObjectDetailsPanel::Tick(float deltaTime) {
    controller.Tick(deltaTime);
}