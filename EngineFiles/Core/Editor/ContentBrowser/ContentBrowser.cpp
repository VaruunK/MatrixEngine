#include "ContentBrowser.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"
#include "Core/Structs/Appstate.hpp"
#include <imgui_impl_sdlgpu3.h>
#include <nfd.hpp>
#include <filesystem>
#include <Core/Assets/AssetLoader/AssetLoader.hpp>

ContentBrowser::ContentBrowser(Appstate& appstate) : appstate(appstate), controller(this) {
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
        
        if (ImGui::Button("Import +")) {
            nfdu8char_t* outPath = nullptr;
            nfdu8filteritem_t filters[2] = { { "Mesh", "obj,fbx,FBX,gltf,GLTF" }, { "Texture", "png,jpg" } };
            nfdopendialogu8args_t args = {};
            args.title = "Import Asset";
            args.filterList = filters;
            args.filterCount = 2;
            
            nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
            if (result == NFD_OKAY) {
                printf("Selected file: %s\n", outPath);

                std::filesystem::path filePath = outPath;

                std::string meshExtensions[] = { ".fbx", ".FBX", ".obj", ".gltf", ".GLTF" };
                std::string textureExtensions[] = { ".png", ".jpeg", ".jpg" };

                AssetType assetType = NONE;

                for (auto& extension : meshExtensions) {
                    if (filePath.extension().string().compare(extension) == 0) {
                        assetType = MESH;
                    }
                }

                if (assetType == NONE) {
                    for (auto& extension : textureExtensions) {
                        if (filePath.extension().string().compare(extension) == 0) {
                            assetType = TEXTURE;
                        }
                    }
                }

                switch (assetType) {
                    case MESH:
                        AssetLoader::Get().WriteMesh(filePath, AssetLoader::Get().ImportMesh(filePath));
                        break;
                    case TEXTURE:
                        AssetLoader::Get().ImportTexture(filePath);
                        break;
                    default:
                        break;
                }

                NFD_FreePathU8(outPath);
                outPath = nullptr;
            }
            else if (result == NFD_CANCEL) {
                printf("User cancelled.\n");
            }
            else {
                printf("Error: %s\n", NFD_GetError());
            }
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PopStyleVar();

        if (lockPopup) {
            ImGui::OpenPopup("my_select_popup");
        }
        RenderLockPopup();

        if (ImGui::TreeNode("Content")) {            
            RenderContentFolder(appstate.gamePath.string() + "\\Content");
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
