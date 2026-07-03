#include "Engine.hpp"
#include "Core/Editor/ProjectSelector/ProjectSelector.hpp"
#include "Core/Editor/ProjectLoader/ProjectLoader.hpp"
#include "Core/Editor/Editor.hpp"
#include "Core/Game/Game.hpp"
#include "Core/GameObject/World/World.hpp"
#include "Core/GameObject/World/Level/Level.hpp"
#include "Core/ShaderManager/ShaderManager.hpp"
#include "Core/Structs/AssetStructs.hpp"
#include "Core/Assets/AssetLoader/AssetLoader.hpp"
#include "Core/Assets/DefaultAssets/DefaultAssets.hpp"
#include "Core/GameObject/Entity/Agent/Agent.hpp"
#include "Core/GameObject/Component/SpriteComponent/SpriteComponent.hpp"
#include "Core/GameObject/Component/MeshComponent/MeshComponent.hpp"
#include "Core/Event/EventBUS/EngineEventBUS.hpp"
#include <imgui_impl_sdl3.h>
#include <nfd.hpp>
#include <cstdio>
#include <iostream>
#include <SDL3_image/SDL_image.h>
#include <SDL3/sdl_gpu.h>
#include <imgui_impl_sdlgpu3.h>
#include <d3d12sdklayers.h>

Engine::Engine() {
    if (!SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
            SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
            throw std::runtime_error("Failed to initialize SDL Video");
        }
    }

    appstate = {
        .device = CreateDevice(),
        .window = CreateSDLWindow()
    };

    NFD_Init();
    
    icon = IMG_Load("Engine.png");

    if (!icon) {
        SDL_Log("couldn't load icon: %s", SDL_GetError());
    } else {
        SDL_SetWindowIcon(appstate.window, icon);
        SDL_DestroySurface(icon);
    }

    if (!SDL_ClaimWindowForGPUDevice(appstate.device, appstate.window)) {
        SDL_Log("Failed to claim window: %s", SDL_GetError());
        // throw runtime error
    }

    if (!SDL_ShaderCross_Init()) {
        SDL_Log("ERROR: SDL_ShaderCross_Init failed!");
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
        // throw runtime error
    }

    if (!ImGui_ImplSDL3_InitForSDLGPU(appstate.window)) {
        SDL_Log("Failed to initialize ImGui SDL3 backend");
        // throw runtime error
    }

    ShaderManager::Init(appstate);

    AssetLoader::Init(appstate);
    
    projectSelector = new ProjectSelector(appstate);
    projectLoader = new ProjectLoader(appstate);

    &ID3D12DebugDevice::ReportLiveDeviceObjects;
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

SDL_Window* Engine::CreateSDLWindow() {
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

    projectSelector->Run();
    std::string& selectedGamePath = projectSelector->selectedGamePathString;
    std::string& selectedGameName = projectSelector->selectedGameNameString;
    if (selectedGamePath.compare("") == 0) {
        Shutdown();
        return 0;
    }

    Game* game = projectLoader->LoadProject(selectedGamePath, selectedGameName);

    appstate.gamePath = selectedGamePath;

    std::cout << selectedGamePath << std::endl;

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

    Mesh* freddy = AssetLoader::Get().ImportMesh("Content/freddy.gltf");
    Material* freddyMat = new Material{};
    freddyMat->baseColor = AssetLoader::Get().ImportTexture("Content/freddy.png");
    freddy->material = freddyMat;

    Mesh* mogus = AssetLoader::Get().ImportMesh("Content/mogus/mogus.fbx");
    Material* mogusMat = new Material{};
    mogusMat->baseColor = AssetLoader::Get().ImportTexture("Content/mogus/mogus.jpg");
    mogusMat->normal = AssetLoader::Get().ImportTexture("Content/mogus/textures/normal.jpg");
    mogus->material = mogusMat;

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

        agent1->SetRotation(glm::vec3(agent1->GetRotation().x, agent1->GetRotation().y + 1, agent1->GetRotation().z));
        agent2->SetRotation(glm::vec3(agent2->GetRotation().x, agent2->GetRotation().y + 1, agent2->GetRotation().z));
    }
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    Shutdown();
    return 0;
}

void Engine::Shutdown() {
    if (projectSelector) {
        delete projectSelector;
        projectSelector = nullptr;
    }

    if (editor) {
        delete editor;
        editor = nullptr;
    }

    if (game) {
        delete game;
        game = nullptr;
    }

    AssetLoader::Shutdown();
    ShaderManager::Shutdown();

    if (icon) {
        SDL_DestroySurface(icon);
    }
    
    NFD_Quit();
    
    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui::DestroyContext();
    SDL_ShaderCross_Quit();
    SDL_DestroyGPUDevice(appstate.device);
    SDL_DestroyWindow(appstate.window);
    SDL_Quit();
}