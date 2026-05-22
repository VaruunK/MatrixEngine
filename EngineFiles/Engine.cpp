#include <iostream>
#include <cstdio>
#include "Engine.hpp"
#include "World/World.hpp"
#include "World/Level/Level.hpp"
#include "Core/Render/Renderer.hpp"
//#include "Entity/Static/Static.hpp"
#include "Entity/Agent/Agent.hpp"
#include "Entity/Component/SpriteComponent/SpriteComponent.hpp"
//#include "Entity/Component/PhysicsComponent/PhysicsComponent.hpp"

Engine::Engine() {
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
            throw std::runtime_error("Failed to initialize SDL Video");
        }
    }

    CreateDevice();

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
    Agent* agent = nullptr;

    bool vel = true;

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
            }

            if (state[SDL_SCANCODE_SPACE]) {
                agent = level->SpawnFromClass<Agent>();
                agent->GetComponent<SpriteComponent>()->SetTexture("Content/square_red.png");
            }
            if (state[SDL_SCANCODE_W]) {
                if (agent && agent->GetComponent<SpriteComponent>()) {
                    auto spriteComp = agent->GetComponent<SpriteComponent>();
                    spriteComp->SetComponentLocation(glm::vec3(spriteComp->GetComponentLocation().x + 0.01,
                        spriteComp->GetComponentLocation().y, spriteComp->GetComponentLocation().z));
                }
        
            }
            if (state[SDL_SCANCODE_E]) {
                if (agent && agent->GetComponent<SpriteComponent>()) {
                    auto spriteComp = agent->GetComponent<SpriteComponent>();
                    spriteComp->SetComponentRotation(glm::vec3(spriteComp->GetComponentRotation().x,
                        spriteComp->GetComponentRotation().y, spriteComp->GetComponentRotation().z + 10));
                }
            }

            if (state[SDL_SCANCODE_R]) {
                if (agent && agent->GetComponent<SpriteComponent>()) {
                    auto spriteComp = agent->GetComponent<SpriteComponent>();
                    spriteComp->SetComponentScale(glm::vec3(spriteComp->GetComponentScale().x * 1.5,
                        spriteComp->GetComponentScale().y * 1.5, spriteComp->GetComponentScale().z));
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