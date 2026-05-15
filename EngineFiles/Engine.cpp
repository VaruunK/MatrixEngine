#include <iostream>
#include <cstdio>
#include "Engine.hpp"
#include "World/World.hpp"
#include "World/Level/Level.hpp"
#include "Core/Render/Renderer.hpp"
//#include "Entity/Static/Static.hpp"
//#include "Entity/Agent/Agent.hpp"
#include "Entity/Component/ImageComponent/ImageComponent.hpp"
#include "Entity/Component/PhysicsComponent/PhysicsComponent.hpp"

Engine::Engine() {
    world = make_unique<World>();
    tickManager = make_unique<TickManager>();
    renderer = make_unique<Renderer>();
    physicsManager = make_unique<PhysicsManager>();
}

int Engine::Run() {

    if (!renderer->Initialize("Matrix Engine", "Engine.png", 1080, 720)) {
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
    threads.emplace_back(&PhysicsManager::Run, physicsManager.get(), MAX_PHYSICS_FRAMES);
    world->Start();

    Uint64 frequency = SDL_GetPerformanceFrequency();
    Uint64 lastCounter = SDL_GetPerformanceCounter();
    //Agent* agent = nullptr;

    bool vel = true;

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
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                /*agent = level->SpawnFromClass<Agent>();
                agent->GetComponent<ImageComponent>()->SetTexture("C:/development/Engine/media/square_red.png");*/
            }
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                //PhysicsComponent* pc = agent->GetComponent<PhysicsComponent>();
                /*if (vel) {
                    pc->ApplyForce(vec2f(10000.0f, 0.0f));
                    vel = false;
                } else {
                    pc->ApplyForce(vec2f(-10000.0f, 0.0f));
                    vel = true;
                }*/
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