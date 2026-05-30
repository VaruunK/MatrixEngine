#include <iostream>
#include <cstdio>
#include "Engine.hpp"

#include "Core/GameObject/World/Level/Level.hpp"
#include "Core/Assets/AssetStructs.hpp"
#include "Core/Assets/DefaultAssets/DefaultAssets.hpp"
//#include "Entity/Static/Static.hpp"
#include "Core/GameObject/Entity/Agent/Agent.hpp"
#include "Core/GameObject/Entity/Component/SpriteComponent/SpriteComponent.hpp"
#include "Core/GameObject/Entity/Component/MeshComponent/MeshComponent.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

//#include "Entity/Component/PhysicsComponent/PhysicsComponent.hpp"


/*
    redoing tick, level, and world system. Gameobjects should have reference to their level and through level have access to world.
    World should not be a game object but should tick, can be some non game object class?
    levels, worlds, and gameobjects / entities should probably be shared pointers?
    debate creating a custom memory system? using tree traversal? need pool allocator as well probably.
    components should be / remain unique pointers.
    World should own a tick manager, engine should have a different(private?) tick manager that ticks world as well as other objects.
*/

Engine::Engine() {
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
            throw std::runtime_error("Failed to initialize SDL Video");
        }
    }

    CreateDevice();

    assetLoader = new AssetLoader();
    windowManager = new WindowManager(device.get());
    // physicsManager = std::make_unique<PhysicsManager>();
    world = new World();
    renderer = new Renderer(device.get());
    viewportCamera = new ViewportCamera();
    viewportController = new ViewportController();
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

int Engine::Run() {
    if (!renderer->Initialize()) {
        renderer->Shutdown();
        return -1;
    }

    Level* level = world->Initialize("StartingLevel");

    // Adding an entity before world start
    //Static* staticGo = level->AddEntityToLevel<Static>();

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

    viewportController->Start(viewportCamera);

    agent1 = level->SpawnFromClass<Agent>();
    meshComponent = agent1->AddComponent<MeshComponent>();
    meshComponent->SetMesh(mesh);

    // Mesh* mesh2 = DefaultCube();

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

            if (event.type == SDL_EVENT_QUIT) {
                running.store(false);
                break;
            }

            if (event.type = SDL_EVENT_WINDOW_RESIZED) {
                renderer->resized = true;
            }
        }
        world->Tick(deltaSeconds);
        viewportController->Tick(deltaSeconds);

        auto pMatrix = viewportCamera->GetProjectionMatrix();
        auto vMatrix = viewportCamera->GetViewMatrix();

        renderer->Render(pMatrix, vMatrix);
    }
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    renderer->Shutdown();
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