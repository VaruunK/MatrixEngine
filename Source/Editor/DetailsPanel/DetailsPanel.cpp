#include "DetailsPanel.hpp"
#include "Core/GameObject/GameObject.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"
#include "Core/Structs/Transform.hpp"
#include "Core/GameObject/Entity/Entity.hpp"
#include "Core/TypeRegistry/TypeRegistry.hpp"
#include <imgui_impl_sdlgpu3.h>
#include <iostream>

#ifdef MATRIX_EDITOR

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
            ImGui::Text("Class: %s", rc.typeInfo->typeName.c_str());
            ImGui::Text("Parent: %s", rc.parent.c_str());
            ImGui::Separator();
            
            for (auto& access : accessTypes) {
                if (rc.fields.contains(access)) {
                    if (ImGui::TreeNode(access.c_str())) {
                        for (const Field& field : rc.fields.at(access)) {
                            RenderField(field);
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

void DetailsPanel::RenderField(const Field& field) {
    const std::shared_ptr<TypeInfo> typeInfo = field.typeInfo;
    ImGui::Separator();
    
    
    // container type switch, singles, maps, arrays, sets, etc, need to be handled differently
    switch (typeInfo->containerType) {

    case ContainerType::SINGLE: {
            ImGui::Text(typeInfo->typeName.c_str());
            const Reflection& reflection = TypeRegistry::Get(typeInfo->typeName);
            
            switch (reflection.type) {
            case ReflectionType::STRUCT: {
                if (ImGui::TreeNode(field.fieldName.c_str())) {
                    const Struct& info = static_cast<const Struct&>(reflection);
                    RenderStructField(field, info);
                    ImGui::TreePop();
                }
                break;
            }
            case ReflectionType::CLASS: {
                if (ImGui::TreeNode(field.fieldName.c_str())) {
                    const Class& info = static_cast<const Class&>(reflection);
                    RenderClassField(field, info);
                    ImGui::TreePop();
                }
                break;
            }
            default:
                break;
            }
        }
        break;

    case ContainerType::MAP: {
        std::shared_ptr<MapTypeInfo> mapInfo = std::static_pointer_cast<MapTypeInfo>(typeInfo);
        void* mapPtr = static_cast<uint8_t*>(static_cast<void*>(detailEntity)) + field.offset;

        if (mapInfo->forEachEntry) {
            
            struct RenderContext {
                DetailsPanel* self;
                const Field* field;
                MapTypeInfo* mapInfo;
            };

            RenderContext ctx{ this, &field, mapInfo.get() };

            mapInfo->forEachEntry(mapPtr, [](const void* keyPtr, void* valPtr, void* userdata) {
                auto* ctx = static_cast<RenderContext*>(userdata);

                if (ctx->mapInfo->valInfo->typeName == "Component*") {
                    const Reflection& reflection = TypeRegistry::Get(ctx->mapInfo->valInfo->typeName);
                    switch (reflection.type) {
                    case ReflectionType::STRUCT: {
                        if (ImGui::TreeNode(ctx->field->fieldName.c_str())) {
                            const Struct& info = static_cast<const Struct&>(reflection);
                            ctx->self->RenderStructField(*ctx->field, info);
                            ImGui::TreePop();
                        }
                        break;
                    }
                    case ReflectionType::CLASS: {
                        if (ImGui::TreeNode(ctx->field->fieldName.c_str())) {
                            const Class& info = static_cast<const Class&>(reflection);
                            ctx->self->RenderClassField(*ctx->field, info);
                            ImGui::TreePop();
                        }
                        break;
                    }
                    default:
                        break;
                    }
                }
                }, &ctx);
        }
        break;
    }
    default:
        break;
    }
}

void DetailsPanel::RenderStructField(const Field& field, const Struct& reflection) {
    
    void* structPtr = static_cast<uint8_t*>(static_cast<void*>(detailEntity)) + field.offset;

    for (auto& member : reflection.members) {
        const std::string& memberName = member.first;
        const std::shared_ptr<TypeInfo> memberType = member.second;
        if (reflection.typeInfo->typeName == "Transform") {
            if (memberType->typeName == "glm::vec3") {
                glm::vec3 vector;
                GetStructMemberValue(reflection, memberName, structPtr, &vector);

                std::string label = "##" + memberName;
                ImGui::Text("%s", (memberName + ": ").c_str());
                ImGui::SameLine();
                if (ImGui::DragFloat3(label.c_str(), &vector.x, 0.1f, -100000.0f, 100000.0f)) {
                    SetStructMemberValue(reflection, memberName, structPtr, &vector);
                    detailEntity->SetTransform(*static_cast<Transform*>(structPtr));
                }
            }
        } else {
            ImGui::Text(member.first.c_str());
        }
    }
}

void DetailsPanel::RenderClassField(const Field& field, const Class& reflection) {
    
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

#endif