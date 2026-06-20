#include "Engine.hpp"
#include "Core/GameObject/World/Level/Level.hpp"
#include "Core/Editor/Render/WorldRenderer/WorldRenderer.hpp"
#include "Core/Structs/AssetStructs.hpp"
#include "Core/Assets/DefaultAssets/DefaultAssets.hpp"
#include "Core/GameObject/Entity/Agent/Agent.hpp"
#include "Core/GameObject/Component/SpriteComponent/SpriteComponent.hpp"
#include "Core/GameObject/Component/MeshComponent/MeshComponent.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"
#include <imgui_impl_sdl3.h>
#include <iostream>
#include <cstdio>
#include <SDL3_image/SDL_image.h>
#include <SDL3/sdl_gpu.h>
#include <nfd.hpp>
#include <pugixml.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <imgui_impl_sdlgpu3.h>

Engine::Engine() {
    if (!SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
            SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
            throw std::runtime_error("Failed to initialize SDL Video");
        }
    }

    appstate = {
        .device = CreateDevice(),
        .window = CreateWindow()
    };

    NFD_Init();

    assetLoader = new AssetLoader(appstate.device);

    SDL_Surface* icon = IMG_Load("Engine.png");

    if (!icon) {
        SDL_Log("couldn't load icon: %s", SDL_GetError());
    }
    else {
        SDL_SetWindowIcon(appstate.window, icon);
        SDL_DestroySurface(icon);
    }

    if (!SDL_ClaimWindowForGPUDevice(appstate.device, appstate.window)) {
        SDL_Log("Failed to claim window: %s", SDL_GetError());
        // throw runtime error
    }

    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplSDLGPU3_InitInfo init_info = {};
    init_info.Device = appstate.device;
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(appstate.device, appstate.window);
    init_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
    init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;
    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;

    if (!ImGui_ImplSDLGPU3_Init(&init_info)) {
        SDL_Log("Failed to initialize ImGui SDLGPU3 backend");
        return;
    }

    if (!ImGui_ImplSDL3_InitForSDLGPU(appstate.window)) {
        SDL_Log("Failed to initialize ImGui SDL3 backend");
        return;
    }
}

SDL_GPUDevice* Engine::CreateDevice() {
    SDL_GPUDevice* device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
        true,
        nullptr
    );

    if (!device) {
        SDL_Log("Failed to create GPU device: %s", SDL_GetError());
        throw std::runtime_error("Failed to create GPU device");
    }

    return device;
}

SDL_Window* Engine::CreateWindow() {
    // SDL_WINDOW_FULLSCREEN
    // SDL_WINDOW_BORDERLESS
    SDL_Window* window = SDL_CreateWindow("Matrix Engine", 1080, 720, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        // throw runtime error
    }
    return window;
}

int Engine::Run() {
    std::string configFilePath = "Matrix.config";

    pugi::xml_document doc;
    pugi::xml_parse_result resultxml = doc.load_file(configFilePath.c_str());
    if (!resultxml) {
        SDL_Log(resultxml.description());
        return -1;
    }
   
    if (pugi::xml_node configuration = doc.child("Configuration")) {
        auto gameDirectory = configuration.child("Directories").child("Games");
        auto gameDirectoryPath = gameDirectory.text().as_string();
        std::cout << gameDirectoryPath << std::endl;
        if (strcmp(gameDirectoryPath, "") == 0) {
            nfdu8char_t* outPath = nullptr;
            nfdpickfolderu8args_t args = {
                .title = "Select Project Directory"
            };

            nfdresult_t result = NFD_PickFolderU8_With(&outPath, &args);

            if (result == NFD_OKAY) {
                printf("Selected folder: %s\n", outPath);
                gameDirectory.text().set(outPath);
                doc.save_file(configFilePath.c_str());
                NFD_FreePathU8(outPath);
            } else if (result == NFD_CANCEL) {
                printf("User cancelled.\n");
            } else {
                printf("Error: %s\n", NFD_GetError());
            }
        }
    }

    game = new Game(appstate);

    std::string levelName = "Mainlevel";
    std::string filePath = "";
    game->Initialize(levelName, filePath);

    editor = new Editor(appstate, game);

    Level* level = game->world.GetLevel("Mainlevel");
    running.store(true);
    // threads.emplace_back(&PhysicsManager::Run, physicsManager.get(), MAX_PHYSICS_FRAMES);
    // world->Start();

    Uint64 frequency = SDL_GetPerformanceFrequency();
    Uint64 lastCounter = SDL_GetPerformanceCounter();
    
    SpriteComponent* spriteComponent = nullptr;

    Mesh* freddy = assetLoader->CreateMesh("Content/freddy.gltf", "Content/freddy.png");
    Mesh* mogus = assetLoader->CreateMesh("Content/mogus/mogus.fbx", "Content/mogus/mogus.jpg");

    Transform transform1 = {
        .location = glm::vec3(0.0f, 0.0f, 0.0f),
        .rotation = glm::vec3(0.0f, 180.0f, 0.0f),
        .scale = glm::vec3(0.1f, 0.1f, 0.1f)
    };

    Transform transform2 = {
        .location = glm::vec3(20.0f, 0.0f, 0.0f),
        .rotation = glm::vec3(0.0f, 180.0f, 0.0f),
        .scale = glm::vec3(10.0f, 10.0f, 10.0f)
    };

    Agent* agent1 = level->SpawnFromClass<Agent>(transform1);
    MeshComponent* a1meshComponent = agent1->AddComponent<MeshComponent>();
    a1meshComponent->SetMesh(mogus);

    Agent* agent2 = level->SpawnFromClass<Agent>(transform2);
    MeshComponent* a2meshComponent = agent2->AddComponent<MeshComponent>();
    a2meshComponent->SetMesh(freddy);

    float scaleMax = 100.0f;
    float scaleMin = 1.0f;

    while (running.load()) {
        Uint64 currentCounter = SDL_GetPerformanceCounter();
        float deltaSeconds = static_cast<float>(currentCounter - lastCounter) / static_cast<float>(frequency);
        lastCounter = currentCounter;

        const float MaxDeltaTime = 1.0f / MAX_FRAMES;
        if (deltaSeconds < MaxDeltaTime)
            deltaSeconds = MaxDeltaTime;
        
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                running.store(false);
                break;
            };
            GEventBUS.ProcessEvent(&event);
        }

        editor->Tick(deltaSeconds);
        editor->Render();

        /*static int grow = 1;

        if (grow == 1) {
            if (agent1->GetTransform().scale.x < scaleMax) {
                agent1->SetScale(glm::vec3(agent1->GetScale().x + 0.1, agent1->GetScale().y + 0.1, agent1->GetScale().z + 0.1));
                agent2->SetScale(glm::vec3(agent2->GetScale().x + 0.1, agent2->GetScale().y + 0.1, agent2->GetScale().z + 0.1));
            }
            else {
                grow = 0;
            }
        }
        else {
            if (agent1->GetTransform().scale.x > scaleMin) {
                agent1->SetScale(glm::vec3(agent1->GetScale().x - 0.1, agent1->GetScale().y - 0.1, agent1->GetScale().z - 0.1));
                agent2->SetScale(glm::vec3(agent2->GetScale().x - 0.1, agent2->GetScale().y - 0.1, agent2->GetScale().z - 0.1));
            }
            else {
                grow = 1;
            }
        }*/

        /*agent1->SetRotation(glm::vec3(agent1->GetRotation().x, agent1->GetRotation().y + 1, agent1->GetRotation().z));
        agent2->SetRotation(glm::vec3(agent2->GetRotation().x, agent2->GetRotation().y + 1, agent2->GetRotation().z));*/
    }
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    NFD_Quit();
    SDL_ShaderCross_Quit();
    SDL_DestroyGPUDevice(appstate.device);
    SDL_DestroyWindow(appstate.window);
    SDL_Quit();
    return 0;
}

int main() {
    // Redirect SDL logs to a file
    FILE* logFile = fopen("engine_log.txt", "w");
    if (logFile) {
        SDL_SetLogOutputFunction([](void* userdata, int category, SDL_LogPriority priority, const char* message) {
            FILE* file = static_cast<FILE*>(userdata);
            fprintf(file, "[%d] %s\n", priority, message);
            fflush(file);
            }, logFile);
    }
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
    // Initialize ShaderCross (REQUIRED for runtime shader compilation!)
    if (!SDL_ShaderCross_Init()) {
        SDL_Log("ERROR: SDL_ShaderCross_Init failed!");
        if (logFile) fclose(logFile);
        return -1;
    }

    int result = Engine::GetEngine().Run();
    if (logFile) {
        fclose(logFile);
    }

    return 0;
}