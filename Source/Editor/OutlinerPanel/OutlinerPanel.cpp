#ifdef MATRIX_EDITOR

#include "OutlinerPanel.hpp"
#include "Core/Game/Game.hpp"
#include "Core/GameObject/World/World.hpp"
#include "Core/GameObject/GameObject.hpp"
#include "Core/GameObject/Entity/Entity.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"
#include "Core/Statics/GameStatics.hpp"
#include "Core/Structs/ReflectionStructs.hpp"
#include "Core/TypeRegistry/TypeRegistry.hpp"
#include <imgui.h>
#include <imgui_impl_sdlgpu3.h>
#include <iostream>

OutlinerPanel::OutlinerPanel(std::unordered_map<std::string, std::unordered_map<Entity*, std::string>>* entityNames, 
    std::set<Entity*>* selectedEntities) : 
    
    entityNames(entityNames), selectedEntities(selectedEntities) {
    
    controller.BindMouseButton(SDL_BUTTON_LEFT,
        [this]() { popup = false; }
    );

    controller.BindMouseButton(SDL_BUTTON_RIGHT,
        [this]() { popup = true; }
    );

    controller.Start();
}

void OutlinerPanel::Render(bool* active) {
    ImGuiWindowFlags contentBarFlags = ImGuiWindowFlags_NoCollapse;
    if (locked) {
        contentBarFlags = contentBarFlags | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    }

    if(ImGui::Begin("Outliner", active, contentBarFlags)) {
        bool isFocused = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
        if (isFocused && !wasFocused) {
            GEventBUS.Notify(EVENT_OUTLINER_PANEL_HOVERED);
        }
        wasFocused = isFocused;

        RenderRightClickPopup();

        if (popup) {
            ImGui::OpenPopup("options_popup");
            popup = false;
        }

        if (!entityNames)
            return;

        static ImGuiSelectionBasicStorage selection;
        
        int count = 0;
        for (const auto& [entityClass, entityNameList] : *entityNames) {
            for (const auto& [entity, name] : entityNameList) {
                count++;
            }
        }

        if(ImGui::BeginChild("##Basket", ImVec2(-FLT_MIN, ImGui::GetFontSize() * 20), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY)) {
            ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect1d;
            ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(flags, selection.Size, count);
            selection.ApplyRequests(ms_io);

            for (const auto& [entityClass, entityNameList] : *entityNames) {
                for (const auto& [entity, name] : entityNameList) {
                    if (const Field* idField = FindFieldByName(entity->GetClass(), "id")) {
                        long long idValue = 0;
                        void* objPtr = static_cast<void*>(entity);
                        entity->GetFieldValue(*idField, objPtr, &idValue);
                        bool selected = selection.Contains(idValue);
                        ImGui::SetNextItemSelectionUserData(idValue);
                        if (ImGui::Selectable(name.c_str(), selected)) {
                            if(selectedEntities->empty()) {
                                selectedEntities->insert(entity);
                            } else {
                                selectedEntities->clear();
                                selectedEntities->insert(entity);
                            }   
                        }
                        ImGui::Separator();
                    }
                }
            }
            ms_io = ImGui::EndMultiSelect();
            selection.ApplyRequests(ms_io);
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void OutlinerPanel::RenderRightClickPopup() {
    if (ImGui::BeginPopup("options_popup")) {

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

void OutlinerPanel::Tick(float deltaTime) {
    controller.Tick(deltaTime);
}

const Field* OutlinerPanel::FindFieldByName(const Class& rc, const std::string& name) {
    for (auto& access : accessTypes) {
        if (rc.fields.contains(access)) {
            for (const Field& field : rc.fields.at(access)) {
                if (field.fieldName == name) {
                    return &field;
                }
            }
        }
    }

    if (rc.parent != "None") {
        const Reflection& parentReflection = TypeRegistry::Get(rc.parent);
        if (parentReflection.type == ReflectionType::CLASS) {
            const Class& parentClass = static_cast<const Class&>(parentReflection);
            return FindFieldByName(parentClass, name);
        }
    }

    return nullptr;
}

#endif