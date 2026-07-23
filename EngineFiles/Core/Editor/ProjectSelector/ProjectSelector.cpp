#include "ProjectSelector.hpp"
#include "Core/Structs/Appstate.hpp"
#include "Core/Structs/FrameData.hpp"
#include "Core/Structs/AssetStructs.hpp"
#include "Core/Assets/AssetLoader/AssetLoader.hpp"
#include <nfd.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <SDL3/SDL_log.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_gpu.h>
#include <glm/ext/vector_float4.hpp>
#include <fstream>

ProjectSelector::ProjectSelector(Appstate& appstate) : appstate(appstate) {
    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    pugi::xml_parse_result resultxml = engineConfigDocument.load_file(configFilePath.c_str());
    if (!resultxml) {
        SDL_Log("Could not open Matrix.config file, %s", resultxml.description());
        // throw runtime exception
    }

    GetGameDirectory();

    if (gameDirectoryString.compare("") != 0) {
        if (!std::filesystem::exists(gameDirectoryString)) {
            SDL_Log("Could not locate path, %s", gameDirectoryString.c_str());
            // throw runtime exception
        }
    }
}

ProjectSelector::~ProjectSelector() {
    gameIcons.clear();
}

void ProjectSelector::Run() {

    running = true;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
                break;
            };
        }
        Render();
    }
}

void ProjectSelector::Render() {
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

        ImGui::SetNextWindowSize(ImVec2(io->DisplaySize.x, io->DisplaySize.y));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGuiWindowFlags selectorFlags = ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        if (ImGui::Begin("Project Selector", nullptr, selectorFlags)) {
            if (gameDirectoryString.compare("") == 0) {
                ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
                ImGui::SetNextWindowSize(ImGui::GetWindowSize());
                if (ImGui::Button("Select a Game Directory")) {
                    SelectGameDirectory();
                }
            } else {
                RenderOptionsButtons();
                
                RenderGameDirectory();
            }

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

void ProjectSelector::GetGameDirectory() {
    std::string gameDirectoryPath = "";
    if (pugi::xml_node configuration = engineConfigDocument.child("Configuration")) {
        gameDirectory = configuration.child("Directories").child("Games");
        gameDirectoryString = gameDirectory.text().as_string();
    }
}

void ProjectSelector::SelectGameDirectory() {
    nfdu8char_t* outPath = nullptr;
    nfdpickfolderu8args_t args = {
        .title = "Select Project Directory"
    };
    nfdresult_t result = NFD_PickFolderU8_With(&outPath, &args);
    if (result == NFD_OKAY) {
        // printf("Selected folder: %s\n", outPath);

        gameDirectory.text().set(outPath);
        gameDirectoryString = std::string(outPath);

        engineConfigDocument.save_file(configFilePath.c_str());
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

void ProjectSelector::RenderGameDirectory() {
    ImGui::SetNextWindowPos(ImVec2(301, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowWidth() - 300, ImGui::GetWindowHeight()));

    ImGuiWindowFlags gameDirectoryFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    if (ImGui::Begin("Games", NULL, gameDirectoryFlags)) {
        // loops through folders in game directory
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(gameDirectoryString)) {
            if (entry.is_directory()) {
                // loops through folders in folder
                for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(entry.path())) {
                    // has .mtrx game file
                    if (file.is_regular_file() && (file.path().extension().string().compare(".mtrx") == 0)) {
                        if (!gameIcons.contains(entry.path())) {
                            auto* tex = AssetLoader::Get().ImportTexture("Engine.png");
                            if (tex && tex->texture) {
                                gameIcons[entry.path()] = (ImTextureID)(intptr_t)tex->texture;
                            }
                            else {
                                SDL_Log("Failed to load Engine.png texture");
                                gameIcons[entry.path()] = NULL;
                            }
                        }
                        RenderGameButton(entry);
                    }
                }
            }
        }
    }
    ImGui::End();
}

void ProjectSelector::RenderGameButton(const std::filesystem::directory_entry& entry) {
    ImGui::PushID(entry.path().string().c_str());

    if (ImGui::InvisibleButton("GameCard", ImVec2(-FLT_MIN, ImGui::GetWindowHeight() * 0.1f))) {
        auto& entryPath = entry.path();
        selectedGameNameString = entryPath.filename().string();
        selectedGamePathString = entryPath.string();
        running = false;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();

    float cardHeight = max.y - min.y;
    float padding = cardHeight * 0.1f;
    float iconSize = cardHeight - (padding * 2.0f);
    float textX = min.x + iconSize + (padding * 2.0f);

    drawList->AddRectFilled(min, max, IM_COL32(60, 60, 60, 255));

    if (gameIcons[entry.path()] != NULL) {
        drawList->AddImage(
            gameIcons[entry.path()],
            ImVec2(min.x + padding, min.y + padding),
            ImVec2(min.x + padding + iconSize, min.y + padding + iconSize)
        );
    }

    float titleY = min.y + cardHeight * 0.2f;
    float subtitleY = min.y + cardHeight * 0.5f;

    drawList->AddText(
        ImVec2(textX, titleY),
        IM_COL32_WHITE,
        entry.path().filename().string().c_str()
    );

    drawList->AddText(
        ImVec2(textX, subtitleY),
        IM_COL32(180, 180, 180, 255),
        std::string("Path: " + entry.path().string()).c_str()
    );

    ImGui::PopID();
    ImGui::Separator();
}

static int FilterGameName(ImGuiInputTextCallbackData* data) {
    char c = (char)data->EventChar;

    if (std::isalnum((unsigned char)c) || c == '_' || c == '-') {
        return 0;
    }
    return 1;
}


void ProjectSelector::RenderOptionsButtons() {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300, ImGui::GetWindowHeight()));

    ImGuiWindowFlags optionsFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    if (ImGui::Begin("Options", NULL, optionsFlags)) {
        if (ImGui::Button("Create New Game", ImVec2(-FLT_MIN, ImGui::GetWindowHeight() * 0.125f))) {
            ImGui::OpenPopup("Create New Game");
        }
        RenderCreateGameModel();
        
        ImGui::Separator();

        if (ImGui::Button("Change Game Directory", ImVec2(-FLT_MIN, ImGui::GetWindowHeight() * 0.125f))) {
            SelectGameDirectory();
        }
    }
    ImGui::End();
}

void ProjectSelector::RenderCreateGameModel() {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    ImGui::SetNextWindowSize(ImVec2(500, 150));

    ImGuiWindowFlags createNewGameFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

    if (ImGui::BeginPopupModal("Create New Game", NULL, createNewGameFlags)) {
        auto windowWidth = ImGui::GetWindowContentRegionMax().x;
        auto textWidth = ImGui::CalcTextSize("Project Name").x;

        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::Text("Project Name");

        const int maxGameNameLength = 64;

        static char newGameName[maxGameNameLength] = "NewGame";

        const float input_width = ImGui::CalcItemWidth();
        ImGui::SetCursorPosX((windowWidth - input_width) * 0.5f);
        ImGui::InputText("##", newGameName, IM_COUNTOF(newGameName),
            ImGuiInputTextFlags_CallbackCharFilter, FilterGameName);

        static std::string errorMessage = "";

        textWidth = ImGui::CalcTextSize(errorMessage.c_str()).x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::Text(errorMessage.c_str());

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            std::string newGamePath = gameDirectoryString;
            newGamePath = newGamePath.append("\\").append(newGameName);

            if (strcmp(newGameName, "") == 0) {
                errorMessage = "Project name cannot be empty";
            }
            else if (std::filesystem::exists(newGamePath)) {
                errorMessage = "File path already exists, choose a different name";
            }
            else {
                errorMessage = "";
                CreateNewGameFiles(newGamePath, newGameName);
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SetItemDefaultFocus();

        ImGui::SameLine();

        ImGui::SetCursorPosX((windowWidth) * 0.75f);
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            errorMessage = "";
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void ProjectSelector::CreateNewGameFiles(std::string& newGamePath, char* newGameName) {
    std::filesystem::create_directory(newGamePath);
    
    std::ofstream ofs(newGamePath + "\\" + newGameName + ".mtrx");
    ofs << "this is some text in the new file\n";
    ofs.close();

    std::ofstream hpp(newGamePath + "\\" + newGameName + ".hpp");
    hpp << "#pragma once\n\n";
    hpp << "#include <Core/Game/Game.hpp>\n";
    hpp << "class " << newGameName << " : public Game {\n";
    hpp << "public:\n";
    hpp << "\tusing Game::Game;\n";
    hpp << "\tvoid Initialize(std::string& name, std::string& iconFilePath) override;\n";
    hpp << "\tvoid Start() override;\n";
    hpp << "\tvoid Tick(float deltaTime) override;\n";
    hpp << "\tvoid Quit() override;\n";
    hpp << "};";
    hpp.close();

    std::ofstream cpp(newGamePath + "\\" + newGameName + ".cpp");
    cpp << "#include \"" << newGameName << ".hpp\"\n\n";
    cpp << "void " << newGameName << "::Initialize(std::string& name, std::string& iconFilePath) {\nGame::Initialize(name, iconFilePath);\n}\n\n";
    cpp << "void " << newGameName << "::Start() {\nGame::Start();\n}\n\n";
    cpp << "void " << newGameName << "::Tick(float deltaTime) {\nGame::Tick(deltaTime);\n}\n\n";
    cpp << "void " << newGameName << "::Quit() {\nGame::Quit();\n}\n\n";
    cpp << "extern \"C\" __declspec(dllexport) Game* CreateGame(Appstate& appstate) {\n";
    cpp << "\treturn new " << newGameName << "(appstate);\n";
    cpp << "}\n";
    cpp.close();

    std::ofstream cmake(newGamePath + "\\" + "CMakeLists.txt");
    cmake << "cmake_minimum_required(VERSION 3.31)\n";
    cmake << "project(" << newGameName << " LANGUAGES C CXX)\n\n";
    cmake << "set(CMAKE_CXX_STANDARD 23)\n\n";
    cmake << "set(MATRIX_ENGINE_INSTALL \"C:/Users/varuu/development/Engine/install\" CACHE PATH \"\")\n\n";

    cmake << "add_library(MatrixEngineLib SHARED IMPORTED GLOBAL)\n";
    cmake << "set_target_properties(MatrixEngineLib PROPERTIES\n";
    cmake << "\tIMPORTED_LOCATION_DEBUG          \"${MATRIX_ENGINE_INSTALL}/Debug/lib/MatrixEngineLib.dll\"\n";
    cmake << "\tIMPORTED_LOCATION_RELEASE        \"${MATRIX_ENGINE_INSTALL}/Release/lib/MatrixEngineLib.dll\"\n";
    cmake << "\tIMPORTED_LOCATION_MINSIZEREL     \"${MATRIX_ENGINE_INSTALL}/Release/lib/MatrixEngineLib.dll\"\n";
    cmake << "\tIMPORTED_LOCATION_RELWITHDEBINFO \"${MATRIX_ENGINE_INSTALL}/Release/lib/MatrixEngineLib.dll\"\n";
    cmake << "\tIMPORTED_IMPLIB_DEBUG            \"${MATRIX_ENGINE_INSTALL}/Debug/lib/MatrixEngineLib.lib\"\n";
    cmake << "\tIMPORTED_IMPLIB_RELEASE          \"${MATRIX_ENGINE_INSTALL}/Release/lib/MatrixEngineLib.lib\"\n";
    cmake << "\tIMPORTED_IMPLIB_MINSIZEREL       \"${MATRIX_ENGINE_INSTALL}/Release/lib/MatrixEngineLib.lib\"\n";
    cmake << "\tIMPORTED_IMPLIB_RELWITHDEBINFO   \"${MATRIX_ENGINE_INSTALL}/Release/lib/MatrixEngineLib.lib\"\n";
    cmake << "\tINTERFACE_INCLUDE_DIRECTORIES    \"$<$<CONFIG:Debug>:${MATRIX_ENGINE_INSTALL}/Debug/include/MatrixEngine;${MATRIX_ENGINE_INSTALL}/Debug/include>$<$<CONFIG:Release>:${MATRIX_ENGINE_INSTALL}/Release/include/MatrixEngine;${MATRIX_ENGINE_INSTALL}/Release/include>\"\n";
    cmake << "\tINTERFACE_LINK_LIBRARIES         \"$<$<CONFIG:Debug>:${MATRIX_ENGINE_INSTALL}/Debug/lib/SDL3d.lib>$<$<CONFIG:Release>:${MATRIX_ENGINE_INSTALL}/Release/lib/SDL3.lib>;$<$<CONFIG:Debug>:${MATRIX_ENGINE_INSTALL}/Debug/lib/SDL3_image-staticd.lib>$<$<CONFIG:Release>:${MATRIX_ENGINE_INSTALL}/Release/lib/SDL3_image-static.lib>;$<$<CONFIG:Debug>:${MATRIX_ENGINE_INSTALL}/Debug/lib/zlibstaticd.lib>$<$<CONFIG:Release>:${MATRIX_ENGINE_INSTALL}/Release/lib/zlibstatic.lib>;$<$<CONFIG:Debug>:${MATRIX_ENGINE_INSTALL}/Debug/lib/assimp-vc143-mtd.lib>$<$<CONFIG:Release>:${MATRIX_ENGINE_INSTALL}/Release/lib/assimp-vc143-mt.lib>;$<$<CONFIG:Debug>:${MATRIX_ENGINE_INSTALL}/Debug/lib/SDL3_shadercrossd.lib>$<$<CONFIG:Release>:${MATRIX_ENGINE_INSTALL}/Release/lib/SDL3_shadercross.lib>;$<$<CONFIG:Debug>:${MATRIX_ENGINE_INSTALL}/Debug/lib/pugixmld.lib>$<$<CONFIG:Release>:${MATRIX_ENGINE_INSTALL}/Release/lib/pugixml.lib>\"\n";
    cmake << ")\n\n";

    cmake << "file(GLOB_RECURSE SOURCE_FILES \"${CMAKE_SOURCE_DIR}/Source/*.cpp\")\n\n";

    cmake << "add_library(" << newGameName << " SHARED\n";
    cmake << "\t\"" << newGameName << ".cpp\"\n";
    cmake << "\t${SOURCE_FILES}\n";
    cmake << ")\n\n";

    cmake << "target_compile_features(" << newGameName << " PRIVATE cxx_std_23)\n";
    cmake << "target_include_directories(" << newGameName << " PRIVATE ${CMAKE_SOURCE_DIR})\n";
    cmake << "target_link_libraries(" << newGameName << " PRIVATE MatrixEngineLib)\n\n";

    cmake << "add_custom_command(TARGET " << newGameName << " POST_BUILD\n";
    cmake << "\tCOMMAND ${CMAKE_COMMAND} -E copy_if_different\n";
    cmake << "\t\t\"$<$<CONFIG:Debug>:${MATRIX_ENGINE_INSTALL}/Debug/lib/MatrixEngineLib.dll>$<$<CONFIG:Release>:${MATRIX_ENGINE_INSTALL}/Release/lib/MatrixEngineLib.dll>\"\n";
    cmake << "\t\t$<TARGET_FILE_DIR:" << newGameName << ">\n";
    cmake << "\tCOMMAND ${CMAKE_COMMAND} -E copy_if_different\n";
    cmake << "\t\t\"$<$<CONFIG:Debug>:${MATRIX_ENGINE_INSTALL}/Debug/lib/SDL3d.dll>$<$<CONFIG:Release>:${MATRIX_ENGINE_INSTALL}/Release/lib/SDL3.dll>\"\n";
    cmake << "\t\t$<TARGET_FILE_DIR:" << newGameName << ">\n";
    cmake << "\tCOMMAND ${CMAKE_COMMAND} -E copy_if_different\n";
    cmake << "\t\t\"$<$<CONFIG:Debug>:${MATRIX_ENGINE_INSTALL}/Debug/lib/SDL3_shadercrossd.dll>$<$<CONFIG:Release>:${MATRIX_ENGINE_INSTALL}/Release/lib/SDL3_shadercross.dll>\"\n";
    cmake << "\t\t$<TARGET_FILE_DIR:" << newGameName << ">\n";
    cmake << "\tCOMMENT \"Copying runtime DLLs\"\n";
    cmake << ")\n";

    cmake.close();

    std::filesystem::create_directory(newGamePath + "\\Source");
    std::filesystem::create_directory(newGamePath + "\\Content");
    std::filesystem::create_directory(newGamePath + "\\build");
}
