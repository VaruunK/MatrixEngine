#include "World.hpp"
#include "Engine.hpp"
#include "Core/TickManager/TickManager.hpp"
#include "Core/GameObject/World/Level/Level.hpp"
#include "Core/GameObject/World/WorldRenderer/WorldRenderer.hpp"
#include "Core/GameObject/Component/MeshComponent/MeshComponent.hpp"
#include "Core/GameObject/Component/SpriteComponent/SpriteComponent.hpp"
#include "Core/Structs/View.hpp"
#include "Core/Structs/FrameData.hpp"
#include <SDL3/SDL.h>
#include <iostream>

World::World() : GameObject() {
    canTick = true;
    deltaSeconds = 0;
    running.store(false);
}

Level* World::Initialize(const std::string& startLevelName, SDL_GPUDevice* device) {
    renderer = new WorldRenderer(device);

    Level* level = CreateInitialLevel(startLevelName);

    if (!renderer->Initialize()) {
        SDL_Log("Failed to start World Renderer");
        renderer->Shutdown();
        return nullptr;
    }

    return level;
}

void World::Start() {
    GameObject::Start();
    for (Level* level : loadedLevels) {
        level->Load();
    }
    
    for (Level* level : loadedLevels) {
        const std::vector<Entity*> entities = level->GetAllEntities();
        for (Entity* entity : entities) {
            entity->Start();
        }
    }
}

void World::Tick(FrameData& frame) {
    GameObject::Tick(frame.deltaTime);
    for (Level* level : loadedLevels) {
        const std::vector<Entity*> entities = level->GetAllEntities();
        for (Entity* entity : entities) {

        }
    }
    renderer->Render(frame);
}

void World::DestroyGameObject() {
    GameObject::DestroyGameObject();
    renderer->Shutdown();
}

Level* World::GetLevel(const std::string& levelName) {
    auto lvl = levels.find(levelName);
    if (lvl != levels.end()) {
        return lvl->second;
    }
    SDL_Log("Couldn't find Level with name: %s", levelName.c_str());
    // throw runtime error?
    return nullptr;
}

Level* World::CreateLevel(const std::string& levelName) {
    if (GetLevel(levelName)) {
        SDL_Log("Level %s already exists", levelName.c_str());
        // throw runtime error
        return nullptr;
    }

    Level* level = new Level(this);

    levels.emplace(levelName, level);
    return level;
}

Level* World::CreateInitialLevel(const std::string& startLevelName) {
    auto level = new Level(this);

    mainLevel = level;
    levels.emplace(startLevelName, level);
    return mainLevel;
}

void World::RegisterMesh(MeshComponent* mesh) {
    renderer->RegisterMesh(mesh);
}

void World::DeregisterMesh(MeshComponent* mesh) {
    renderer->DeregisterMesh(mesh);
}

void World::RegisterSprite(SpriteComponent* sprite) {
    renderer->RegisterSprite(sprite);
}

void World::DeregisterSprite(SpriteComponent* sprite) {
    renderer->DeregisterSprite(sprite);
}
