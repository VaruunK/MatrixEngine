#include "DetailsPanel.hpp"
#include "Core/GameObject/GameObject.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"
#include "Core/Structs/Transform.hpp"
#include "Core/GameObject/Entity/Entity.hpp"
#include "Core/TypeRegistry/TypeRegistry.hpp"
#include <imgui_impl_sdlgpu3.h>
#include <array>
#include <iostream>

DetailsPanel::DetailsPanel() {
    controller.BindMouseButton(SDL_BUTTON_LEFT,
        [this]() { popup = false; }
    );

    controller.BindMouseButton(SDL_BUTTON_RIGHT,
        [this]() { popup = true; }
    );

    controller.Start();
}

void DetailsPanel::SetEntityToView(Entity* entity) {
	detailEntity = entity;
}

void DetailsPanel::Render(bool* active) {
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

        if (detailEntity) {
            const Class& rc = detailEntity->GetClass();
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

                            const Reflection& reflection = TypeRegistry::Get(field.typeName);

                            switch (reflection.type) {
                            case ReflectionType::STRUCT: {
                                const Struct& info = static_cast<const Struct&>(reflection);

                                if (ImGui::TreeNode(field.name.c_str())) {
                                    void* structPtr = static_cast<uint8_t*>(static_cast<void*>(detailEntity)) + field.offset;

                                    for (auto& member : info.members) {
                                        const std::string& memberName = member.first;
                                        const std::string& memberType = member.second;
                                        if (info.name == "Transform") {
                                            if (memberType == "glm::vec3") {
                                                glm::vec3 vector;
                                                GetStructMemberValue(info, memberName, structPtr, &vector);

                                                std::string label = "##" + memberName;
                                                ImGui::Text("%s", (memberName + ": ").c_str());
                                                ImGui::SameLine();
                                                if (ImGui::DragFloat3(label.c_str(), &vector.x, 0.1f, -100000.0f, 100000.0f)) {
                                                    SetStructMemberValue(info, memberName, structPtr, &vector);
                                                    detailEntity->SetTransform(*static_cast<Transform*>(structPtr));
                                                }
                                            }
                                        }
                                    }

                                    ImGui::TreePop();
                                }
                                break;
                            }
                            case ReflectionType::CLASS: {
                                const Class& info = static_cast<const Class&>(reflection);
                                break;
                            }
                            default:
                                break;
                            }
                        }
                        ImGui::TreePop();
                    }
                }
            }
        }
        else {
            ImGui::Text("Class: None");
        }
    }
    ImGui::End();
}

void DetailsPanel::RenderRightClickPopup() {
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

void DetailsPanel::Tick(float deltaTime) {
    controller.Tick(deltaTime);
}