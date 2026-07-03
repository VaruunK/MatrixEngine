#include "ContentBrowser.hpp"
#include "Core/Structs/Appstate.hpp"
#include "Core/Assets/AssetLoader/AssetLoader.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"
#include <imgui_impl_sdlgpu3.h>
#include <nfd.hpp>
#include <filesystem>
#include <iostream>
#include <SDL3/SDL_log.h>

ContentBrowser::ContentBrowser(Appstate& appstate) : appstate(appstate) {
    currentPath = appstate.gamePath.string() + "\\Content";
    hoveredItem = "";
    
    controller.BindMouseButton(SDL_BUTTON_LEFT,
        [this]() { 
            popup = false;
        });

    controller.BindMouseButton(SDL_BUTTON_RIGHT,
        [this]() {
            popup = true;
        });

    controller.BindKey(SDL_SCANCODE_F2,
        [this]() {
            if (!hoveredItem.empty()) {
                f2RenamePending = true;
            }
        });
    
    controller.Start();
}

void ContentBrowser::Render(bool* active) {
    if (f2RenamePending) {
        if (!hoveredItem.empty()) {
            selectedItem = hoveredItem;
            SetRenamePath();
        }
        f2RenamePending = false;
    }

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

        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2 / 7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2 / 7.0f, 0.8f, 0.8f));

        
        if (ImGui::Button("Import +")) {
            ImportNewAsset();
        }

        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();
        RenderBreadcrumb();

        RenderContentFolder(currentPath);

        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
            selectedItem.clear();
        }

        RenderRightClickPopup();
        RenderDeleteFileModel();

        if (popup) {
            ImGui::OpenPopup("options_popup");
            popup = false;
        }

        if (deleteFilePopup) {
            ImGui::OpenPopup("Delete File Conformation");
            deleteFilePopup = false;
        }
        RenderImportMeshModel();
    }
    ImGui::End();
}

static int FilterAssetName(ImGuiInputTextCallbackData* data) {
    char c = (char)data->EventChar;

    if (std::isalnum((unsigned char)c) || c == '_' || c == '-') {
        return 0;
    }
    return 1;
}


void ContentBrowser::ImportNewAsset() {
    nfdu8char_t* outPath = nullptr;

    nfdu8filteritem_t filters[2] = {
        { "Mesh", "obj,fbx,FBX,gltf,GLTF" },
        { "Texture", "png,jpg" }
    };

    nfdopendialogu8args_t args = {};
    args.title = "Import Asset";
    args.filterList = filters;
    args.filterCount = 2;

    nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);

    if (result == NFD_OKAY) {
        importFile = outPath;
        NFD_FreePathU8(outPath);

        importAssetType = NONE;

        static const std::string meshExtensions[] = { ".fbx", ".FBX", ".obj", ".gltf", ".GLTF" };

        static const std::string textureExtensions[] = { ".png", ".jpg", ".jpeg" };

        for (const auto& ext : meshExtensions) {
            if (importFile.extension() == ext) {
                importAssetType = MESH;
                break;
            }
        }

        if (importAssetType == NONE) {
            for (const auto& ext : textureExtensions) {
                if (importFile.extension() == ext) {
                    importAssetType = TEXTURE;
                    break;
                }
            }
        }

        std::snprintf(importAssetName, sizeof(importAssetName), "%s", importFile.stem().string().c_str());

        if (importAssetType == MESH) {
            ImGui::OpenPopup("Import Mesh Settings");
        }
        else if (importAssetType == TEXTURE) {
            AssetLoader::Get().ImportTexture(importFile);
        }
    }
    else if (result == NFD_ERROR) {
        printf("Error: %s\n", NFD_GetError());
    }
}

void ContentBrowser::RenderRightClickPopup() {
    if (ImGui::BeginPopup("options_popup")) {

        if (ImGui::Selectable("New Folder")) {
            int i = 0;
            std::filesystem::path folderPath;

            do {
                folderPath = currentPath / (i == 0 ? "NewFolder" : "NewFolder(" + std::to_string(i) + ")");
                ++i;
            } while (std::filesystem::exists(folderPath));

            std::filesystem::create_directory(folderPath);
        }

        if (!selectedItem.empty()) {
            ImGui::Separator();
            if (ImGui::Selectable("Rename")) {
                SetRenamePath();
            }
            if (ImGui::Selectable("Delete")) {
                SetDeletePath();
            }
        }

        ImGui::Separator();
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

void ContentBrowser::RenderImportMeshModel() {

    ImGui::SetNextWindowPos(
        ImGui::GetMainViewport()->GetCenter(),
        ImGuiCond_Appearing,
        ImVec2(0.5f, 0.5f));

    ImGui::SetNextWindowSize(ImVec2(500, 150));

    if (ImGui::BeginPopupModal("Import Mesh Settings", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
        hoveredItem = "";
        float windowWidth = ImGui::GetWindowContentRegionMax().x;
        float textWidth = ImGui::CalcTextSize("Asset Name").x;

        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::Text("Asset Name");

        float inputWidth = ImGui::CalcItemWidth();
        ImGui::SetCursorPosX((windowWidth - inputWidth) * 0.5f);

        ImGui::InputText(
            "##AssetName",
            importAssetName,
            IM_ARRAYSIZE(importAssetName),
            ImGuiInputTextFlags_CallbackCharFilter,
            FilterAssetName);

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            std::filesystem::path writePath =
                currentPath / (std::string(importAssetName) + ".asset");

            AssetLoader::Get().WriteMesh(
                writePath,
                AssetLoader::Get().ImportMesh(importFile));

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void ContentBrowser::RenderDeleteFileModel() {
    ImGui::SetNextWindowPos(
        ImGui::GetMainViewport()->GetCenter(),
        ImGuiCond_Appearing,
        ImVec2(0.5f, 0.5f));

    ImGui::SetNextWindowSize(ImVec2(500, 150));

    if (ImGui::BeginPopupModal("Delete File Conformation", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
        float windowWidth = ImGui::GetWindowContentRegionMax().x;
        
        char confirmText[256];
        std::snprintf(confirmText, sizeof(confirmText),
            "Are you sure you want to delete %s?", deleteFile.filename().string().c_str());

        float textWidth = ImGui::CalcTextSize(confirmText).x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::Text("%s", confirmText);
        
        std::string warningText;
        auto status = std::filesystem::status(deleteFile);
        if (std::filesystem::is_directory(status)) {
            warningText = "All assets in the directory will also be deleted";
            textWidth = ImGui::CalcTextSize(warningText.c_str()).x;
            ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
            ImGui::Text(warningText.c_str());
        } else {
            warningText = "Any assets referencing this will be affected";
            textWidth = ImGui::CalcTextSize(warningText.c_str()).x;
            ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
            ImGui::Text(warningText.c_str());
        }
        
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();

            std::error_code ec;
            std::filesystem::remove_all(deleteFile, ec);
            if (ec) {
                SDL_Log("Delete failed: %s", ec.message().c_str());
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void ContentBrowser::SetRenamePath() {
    renamingFile = selectedItem;
    std::snprintf(renameBuffer, sizeof(renameBuffer), "%s",
        selectedItem.filename().string().c_str());
    renameFocusRequested = true;
    if (popup) {
        ImGui::CloseCurrentPopup();
    }
}

void ContentBrowser::SetDeletePath() {
    deleteFile = selectedItem;
    if (popup) {
        ImGui::CloseCurrentPopup();
    }
    deleteFilePopup = true;
}

void ContentBrowser::RenderBreadcrumb() {
    std::vector<std::filesystem::path> breadcrumb;

    std::filesystem::path p = currentPath;
    std::filesystem::path root = appstate.gamePath / "Content";

    while (true) {
        breadcrumb.push_back(p);

        if (p == root) {
            break;
        }

        p = p.parent_path();
    }

    std::reverse(breadcrumb.begin(), breadcrumb.end());

    for (size_t i = 0; i < breadcrumb.size(); i++) {
        if (i > 0) {
            ImGui::SameLine();
            ImGui::Text(">");
            ImGui::SameLine();
        }

        std::string name = breadcrumb[i].filename().string();

        // Root folder has empty filename on some paths
        if (breadcrumb[i] == root) {
            name = "Content";
        }

        ImGui::PushID(("N" + breadcrumb[i].string()).c_str());
        if (ImGui::SmallButton(name.c_str())) {
            hoveredItem.clear();
            selectedItem.clear();
            currentPath = breadcrumb[i];
        }
        ImGui::PopID();

        ImGui::SameLine();
    }

    ImGui::NewLine();
}

void ContentBrowser::RenderContentFolder(const std::filesystem::path& path) {
    hoveredItem.clear();

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_directory()) {

            std::string name = entry.path().filename().string();
            bool isRenamingThis = (entry.path() == renamingFile);

            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, (ImVec4)ImColor::HSV(4.0f / 7.0f, 0.7f, 0.9f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, (ImVec4)ImColor::HSV(4.0f / 7.0f, 0.8f, 1.0f));
            ImGui::PushID(("B" + entry.path().string()).c_str());

            if (isRenamingThis) {
                RenderRenaming(entry.path(), name);

            } else {
                ImVec2 textSize = ImGui::CalcTextSize(name.c_str());
                if(ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick, textSize)) {
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        selectedItem.clear();
                        currentPath = entry.path();
                    }
                }

                bool isHovered = ImGui::IsItemHovered();
                bool isRightClicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);

                if (isHovered) {
                    hoveredItem = entry.path();
                }

                if (isRightClicked) {
                    selectedItem = entry.path();
                    ImGui::OpenPopup("options_popup");
                }
            }

            ImGui::PopID();
            ImGui::PopStyleColor(2);
        } else {
            ImGui::Text("%s", entry.path().filename().string().c_str());
        }
    }
}

void ContentBrowser::RenderRenaming(const std::filesystem::path& path, std::string& name) {
    if (renameFocusRequested) {
        ImGui::SetKeyboardFocusHere();
        renameFocusRequested = false;
    }

    ImGui::SetNextItemWidth(ImGui::CalcTextSize(name.c_str()).x + 40.0f);

    bool enterPressed = ImGui::InputText(
        "##RenameField",
        renameBuffer,
        IM_ARRAYSIZE(renameBuffer),
        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCharFilter,
        FilterAssetName);

    bool lostFocus = ImGui::IsItemDeactivated();

    if (enterPressed || lostFocus) {
        std::string newName = renameBuffer;

        if (!newName.empty() && newName != path.filename().string()) {
            std::filesystem::path newPath = path.parent_path() / newName;

            if (!std::filesystem::exists(newPath)) {
                std::error_code ec;
                std::filesystem::rename(path, newPath, ec);
                if (ec) {
                    SDL_Log("Rename failed: %s", ec.message().c_str());
                }
            }
        }

        renamingFile.clear();
    }

    // Cancel rename on Escape
    if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        renamingFile.clear();
    }
}

void ContentBrowser::Tick(float deltaTime) {
    controller.Tick(deltaTime);
}