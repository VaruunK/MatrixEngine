#include "Engine.hpp"
#include "Core/GameObject/World/Level/Level.hpp"
#include "Core/GameObject/World/WorldRenderer/WorldRenderer.hpp"
#include "Core/Structs/AssetStructs.hpp"
#include "Core/Structs/View.hpp"
#include "Core/Assets/DefaultAssets/DefaultAssets.hpp"
#include "Core/GameObject/Entity/Agent/Agent.hpp"
#include "Core/GameObject/Component/SpriteComponent/SpriteComponent.hpp"
#include "Core/GameObject/Component/MeshComponent/MeshComponent.hpp"
#include "Core/Structs/FrameData.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include <cstdio>
#include <SDL3_image/SDL_image.h>
#include <SDL3/sdl_gpu.h>

Engine::Engine() {
    if (!SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
            SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
            throw std::runtime_error("Failed to initialize SDL Video");
        }
    }

    CreateDevice();
    CreateWindow();

    assetLoader = new AssetLoader(device.get());
    viewport = new Viewport(device.get(), window);
}

void Engine::CreateDevice() {
    device.reset(SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
        true,
        nullptr
    ));

    if (!device.get()) {
        SDL_Log("Failed to create GPU device: %s", SDL_GetError());
        throw std::runtime_error("Failed to create GPU device");
    }
}

void Engine::CreateWindow() {
    // SDL_WINDOW_FULLSCREEN
    // SDL_WINDOW_BORDERLESS
    window = SDL_CreateWindow("Matrix Engine", 1080, 720, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        // throw runtime error
    }

    SDL_Surface* icon = IMG_Load("Engine.png");

    if (!icon) {
        SDL_Log("couldn't load icon: %s", SDL_GetError());
    }
    else {
        SDL_SetWindowIcon(window, icon);
        SDL_DestroySurface(icon);
    }

    if (!SDL_ClaimWindowForGPUDevice(device.get(), window)) {
        SDL_Log("Failed to claim window: %s", SDL_GetError());
        // throw runtime error
    }
}

int Engine::Run() {
    game = new Game();

    game->device = device.get();
    game->window = window;
    std::string levelName = "Mainlevel";
    std::string filePath = "";
    game->Initialize(levelName, filePath);

    viewport->Initialize(game->world->GetWorldRenderer());

    Level* level = game->world->GetLevel("Mainlevel");
    running.store(true);
    // threads.emplace_back(&PhysicsManager::Run, physicsManager.get(), MAX_PHYSICS_FRAMES);
    // world->Start();

    Uint64 frequency = SDL_GetPerformanceFrequency();
    Uint64 lastCounter = SDL_GetPerformanceCounter();
    
    SpriteComponent* spriteComponent = nullptr;
    int cameraMode = 0;

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

        // world->SetDeltaTime(deltaSeconds);
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                running.store(false);
                break;
            }
        }

        // world->Tick(frame);
        viewport->Tick(deltaSeconds);
        viewport->Render();

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
    SDL_ShaderCross_Quit();
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