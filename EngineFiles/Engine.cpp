#include <iostream>
#include <cstdio>
#include "Engine.hpp"
#include "World/World.hpp"
#include "World/Level/Level.hpp"
#include "Core/Render/Renderer.hpp"
#include "Core/Render//RenderStructs.hpp"
//#include "Entity/Static/Static.hpp"
#include "Entity/Agent/Agent.hpp"
#include "Entity/Component/SpriteComponent/SpriteComponent.hpp"
#include "Entity/Component/MeshComponent/MeshComponent.hpp"
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
    Agent* activeAgent = nullptr;
    MeshComponent* meshComponent = nullptr;
    SpriteComponent* spriteComponent = nullptr;
    bool vel = true;

    std::vector<Vertex> vertices = {
        // Front face
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},
        {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},
        {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},

        // Back face
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}},
        {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}},

        // Left face
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}},

        // Right face
        {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}},

        // Top face
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},
        {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},
        {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},

        // Bottom face
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}}
    };

    std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };



    Mesh* mesh = assetLoader.get()->CreateMesh("Content/freddy.gltf", "Content/freddy.png");

    Mesh* mesh2 = new Mesh;
    mesh2->vertices = vertices;
    mesh2->indices = indices;
    mesh2->texture = assetLoader.get()->CreateTexture("Content/square_red.png");

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

            if (state[SDL_SCANCODE_SPACE]) {
                agent1 = level->SpawnFromClass<Agent>();
                meshComponent = agent1->AddComponent<MeshComponent>();
                
                /*agent->GetComponent<SpriteComponent>()->SetTexture("Content/square_red.png");*/
                meshComponent->SetMesh(mesh);
                spriteComponent = nullptr;
                activeAgent = agent1;
            }

            if (state[SDL_SCANCODE_TAB]) {
                agent2 = level->SpawnFromClass<Agent>();
                spriteComponent = agent2->AddComponent<SpriteComponent>();

                spriteComponent->SetTexture(assetLoader.get()->CreateTexture("Content/nanodsa.png"));
                meshComponent = nullptr;
                activeAgent = agent2;
            }

            if (state[SDL_SCANCODE_W]) {
                if (activeAgent && meshComponent) {
                    meshComponent->SetComponentLocation(glm::vec3(meshComponent->GetComponentLocation().x,
                        meshComponent->GetComponentLocation().y + 0.01, meshComponent->GetComponentLocation().z));
                } else if (activeAgent && spriteComponent) {
                    spriteComponent->SetComponentLocation(glm::vec3(spriteComponent->GetComponentLocation().x,
                        spriteComponent->GetComponentLocation().y + 0.01, spriteComponent->GetComponentLocation().z));
                }
            }

            if (state[SDL_SCANCODE_S]) {
                if (activeAgent && meshComponent) {
                    meshComponent->SetComponentLocation(glm::vec3(meshComponent->GetComponentLocation().x,
                        meshComponent->GetComponentLocation().y - 0.01, meshComponent->GetComponentLocation().z));
                }
                else if (activeAgent && spriteComponent) {
                    spriteComponent->SetComponentLocation(glm::vec3(spriteComponent->GetComponentLocation().x,
                        spriteComponent->GetComponentLocation().y - 0.01, spriteComponent->GetComponentLocation().z));
                }
            }

            if (state[SDL_SCANCODE_P]) {
                if (activeAgent && meshComponent) {
                    meshComponent->SetMesh(mesh2);
                }
            }

            if (state[SDL_SCANCODE_E]) {
                if (activeAgent && meshComponent) {
                    meshComponent->SetComponentRotation(glm::vec3(meshComponent->GetComponentRotation().x,
                        meshComponent->GetComponentRotation().y + 10, meshComponent->GetComponentRotation().z));
                } else if (activeAgent && spriteComponent) {
                    spriteComponent->SetComponentRotation(glm::vec3(spriteComponent->GetComponentRotation().x,
                        spriteComponent->GetComponentRotation().y + 10, spriteComponent->GetComponentRotation().z));
                }
            }

            if (state[SDL_SCANCODE_R]) {
                if (activeAgent && meshComponent) {
                    meshComponent->SetComponentScale(glm::vec3(meshComponent->GetComponentScale().x * 1.5,
                        meshComponent->GetComponentScale().y * 1.5, meshComponent->GetComponentScale().z));
                } else if (activeAgent && spriteComponent) {
                    spriteComponent->SetComponentScale(glm::vec3(spriteComponent->GetComponentScale().x * 1.5,
                        spriteComponent->GetComponentScale().y * 1.5, spriteComponent->GetComponentScale().z));
                }
            }
        }

        world->Tick(deltaSeconds);

        renderer->Render();
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