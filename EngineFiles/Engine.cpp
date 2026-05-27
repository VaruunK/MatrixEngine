#include <iostream>
#include <cstdio>
#include "Engine.hpp"
#include "World/World.hpp"
#include "World/Level/Level.hpp"
#include "Core/Render/Renderer.hpp"
#include "Core/Assets/AssetStructs.hpp"
#include "Core/Assets/DefaultAssets/DefaultAssets.hpp"
//#include "Entity/Static/Static.hpp"
#include "Entity/Agent/Agent.hpp"
#include "Entity/Component/SpriteComponent/SpriteComponent.hpp"
#include "Entity/Component/MeshComponent/MeshComponent.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

//#include "Entity/Component/PhysicsComponent/PhysicsComponent.hpp"

Engine::Engine() {
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
            throw std::runtime_error("Failed to initialize SDL Video");
        }
    }

    CreateDevice();

    assetLoader = std::make_unique<AssetLoader>();
    tickManager = std::make_unique<TickManager>();
    windowManager = std::make_unique<WindowManager>(device.get());
    // physicsManager = std::make_unique<PhysicsManager>();
    world = std::make_unique<World>();
    renderer = std::make_unique<Renderer>(device.get());
    viewportCamera = std::make_unique<ViewportCamera>();
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
    
    //ImageComponent* ic = staticGo->GetComponent<ImageComponent>();
    //ic->SetTexture("C:/development/Engine/media/square_green.png");
    //ic->SetScale(2.0f, 2.0f);

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

    Mesh* mesh = assetLoader.get()->CreateMesh("Content/freddy.gltf", "Content/freddy.png");

    Mesh* mesh2 = DefaultCube();

    while (running.load()) {
        Uint64 currentCounter = SDL_GetPerformanceCounter();
        float deltaSeconds = static_cast<float>(currentCounter - lastCounter) / static_cast<float>(frequency);
        lastCounter = currentCounter;

        const float MaxDeltaTime = 1.0f / MAX_FRAMES;
        if (deltaSeconds < MaxDeltaTime)
            deltaSeconds = MaxDeltaTime;

        world->SetDeltaTime(deltaSeconds);
        int numkeys;
        SDL_Event event;
        auto state = std::vector<bool>(SDL_GetKeyboardState(&numkeys), SDL_GetKeyboardState(&numkeys) + numkeys);
        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_EVENT_QUIT) {
                running.store(false);
                break;
            }

            if (event.type = SDL_EVENT_WINDOW_RESIZED) {
                renderer.get()->resized = true;
            }

            if (state[SDL_SCANCODE_TAB]) {
                cameraMode = (cameraMode + 1) % 3;
                std::cout << cameraMode << std::endl;
            }

            if (state[SDL_SCANCODE_1]) {
                agent1 = level->SpawnFromClass<Agent>();
                meshComponent = agent1->AddComponent<MeshComponent>();
                
                /*agent->GetComponent<SpriteComponent>()->SetTexture("Content/square_red.png");*/
                meshComponent->SetMesh(mesh);
            }

            if (state[SDL_SCANCODE_2]) {
                agent2 = level->SpawnFromClass<Agent>();
                spriteComponent = agent2->AddComponent<SpriteComponent>();

                spriteComponent->SetTexture(assetLoader.get()->CreateTexture("Content/nanodsa.png"));
            }

            if (state[SDL_SCANCODE_W]) {
                if (agent1 && meshComponent && cameraMode == 0) {
                    meshComponent->SetComponentLocation(glm::vec3(meshComponent->GetComponentLocation().x,
                        meshComponent->GetComponentLocation().y + 0.01, meshComponent->GetComponentLocation().z));
                } else if (agent2 && spriteComponent && cameraMode == 1) {
                    spriteComponent->SetComponentLocation(glm::vec3(spriteComponent->GetComponentLocation().x,
                        spriteComponent->GetComponentLocation().y + 0.01, spriteComponent->GetComponentLocation().z));
                } else if (cameraMode == 2) {
                    viewportCamera.get()->MoveForward();
                }
            }

            if (state[SDL_SCANCODE_S]) {
                if (agent1 && meshComponent && cameraMode == 0) {
                    meshComponent->SetComponentLocation(glm::vec3(meshComponent->GetComponentLocation().x,
                        meshComponent->GetComponentLocation().y - 0.01, meshComponent->GetComponentLocation().z));

                } else if (agent2 && spriteComponent && cameraMode == 1) {
                    spriteComponent->SetComponentLocation(glm::vec3(spriteComponent->GetComponentLocation().x,
                        spriteComponent->GetComponentLocation().y - 0.01, spriteComponent->GetComponentLocation().z));
                } else if (cameraMode == 2) {
                    viewportCamera.get()->MoveBackward();
                }
            }

            if (state[SDL_SCANCODE_D]) {
                viewportCamera.get()->MoveRight();
            }

            if (state[SDL_SCANCODE_A]) {
                viewportCamera.get()->MoveLeft();
            }

            if (state[SDL_SCANCODE_P]) {
                if (agent1 && meshComponent && cameraMode == 0) {
                    meshComponent->SetMesh(mesh2);
                }
            }

            if (state[SDL_SCANCODE_E]) {
                if (agent1 && meshComponent && cameraMode == 0) {
                    meshComponent->SetComponentRotation(glm::vec3(meshComponent->GetComponentRotation().x,
                        meshComponent->GetComponentRotation().y + 10, meshComponent->GetComponentRotation().z));
                } else if (agent2 && spriteComponent && cameraMode == 1) {
                    spriteComponent->SetComponentRotation(glm::vec3(spriteComponent->GetComponentRotation().x,
                        spriteComponent->GetComponentRotation().y + 10, spriteComponent->GetComponentRotation().z));
                }
            }

            if (state[SDL_SCANCODE_R]) {
                if (agent1 && meshComponent && cameraMode == 0) {
                    meshComponent->SetComponentScale(glm::vec3(meshComponent->GetComponentScale().x * 1.5,
                        meshComponent->GetComponentScale().y * 1.5, meshComponent->GetComponentScale().z));
                } else if (agent2 && spriteComponent && cameraMode == 1) {
                    spriteComponent->SetComponentScale(glm::vec3(spriteComponent->GetComponentScale().x * 1.5,
                        spriteComponent->GetComponentScale().y * 1.5, spriteComponent->GetComponentScale().z));
                }
            }
        }

        world->Tick(deltaSeconds);

        auto pMatrix = viewportCamera.get()->GetProjectionMatrix();
        auto vMatrix = viewportCamera.get()->GetViewMatrix();

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