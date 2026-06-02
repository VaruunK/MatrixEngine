#include "Engine.hpp"
#include "Core/GameObject/World/Level/Level.hpp"
#include "Core/Structs/AssetStructs.hpp"
#include "Core/Structs/View.hpp"
#include "Core/Assets/DefaultAssets/DefaultAssets.hpp"
#include "Core/GameObject/Entity/Agent/Agent.hpp"
#include "Core/GameObject/Component/SpriteComponent/SpriteComponent.hpp"
#include "Core/GameObject/Component/MeshComponent/MeshComponent.hpp"
#include "Core/WindowManager/Window/Window.hpp"
#include "Core/Structs/FrameData.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include <cstdio>

Engine::Engine() {
    if (!SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
            SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
            throw std::runtime_error("Failed to initialize SDL Video");
        }
    }

    CreateDevice();

    assetLoader = new AssetLoader();
    windowManager = new WindowManager(device.get());
    // physicsManager = std::make_unique<PhysicsManager>();
    world = new World();
    viewport = new Viewport(device.get());
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

void Engine::BeginFrame() {
    currentCommandBuffer = SDL_AcquireGPUCommandBuffer(device.get());
    if (!currentCommandBuffer) {
        SDL_Log("Failed to acquire command buffer");
        return;
    }
    Window* window = windowManager->GetMainWindow();
    window->WaitAndAquireGPUSwapchainTexture(
        currentCommandBuffer, &currentSwapchainTexture, nullptr, nullptr);
}

void Engine::EndFrame() {
    if (currentCommandBuffer) {
        SDL_SubmitGPUCommandBuffer(currentCommandBuffer);
        currentCommandBuffer = nullptr;
        currentSwapchainTexture = nullptr;
    }
}

int Engine::Run() {
    viewport->Initialize();
    Level* level = world->Initialize("StartingLevel", device.get());
    running.store(true);
    // threads.emplace_back(&PhysicsManager::Run, physicsManager.get(), MAX_PHYSICS_FRAMES);
    world->Start();

    Uint64 frequency = SDL_GetPerformanceFrequency();
    Uint64 lastCounter = SDL_GetPerformanceCounter();
    Agent* agent1 = nullptr;
    Agent* agent2 = nullptr;
    MeshComponent* meshComponent = nullptr;
    SpriteComponent* spriteComponent = nullptr;
    int cameraMode = 0;

    Mesh* mesh = assetLoader->CreateMesh("Content/freddy.gltf", "Content/freddy.png");

    agent1 = level->SpawnFromClass<Agent>();
    meshComponent = agent1->AddComponent<MeshComponent>();
    meshComponent->SetMesh(mesh);

    Mesh* mesh2 = DefaultCube();

    while (running.load()) {
        Uint64 currentCounter = SDL_GetPerformanceCounter();
        float deltaSeconds = static_cast<float>(currentCounter - lastCounter) / static_cast<float>(frequency);
        lastCounter = currentCounter;

        const float MaxDeltaTime = 1.0f / MAX_FRAMES;
        if (deltaSeconds < MaxDeltaTime)
            deltaSeconds = MaxDeltaTime;

        world->SetDeltaTime(deltaSeconds);
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                running.store(false);
                break;
            }
        }

        BeginFrame();

        if (currentCommandBuffer && currentSwapchainTexture) {

            FrameData frame{};
            frame.commandBuffer = currentCommandBuffer;
            frame.swapchainTexture = currentSwapchainTexture;
            frame.deltaTime = deltaSeconds;
            frame.view = &viewport->GetCameraView();
            
            world->Tick(frame);
            viewport->Render(frame);
        }

        EndFrame();
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