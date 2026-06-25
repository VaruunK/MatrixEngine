#include "World.hpp"
#include "Engine.hpp"
#include "Core/TickManager/TickManager.hpp"
#include "Core/GameObject/World/Level/Level.hpp"
#include "Core/Editor/Render/WorldRenderer/WorldRenderer.hpp"
#include "Core/GameObject/Component/MeshComponent/MeshComponent.hpp"
#include "Core/GameObject/Component/SpriteComponent/SpriteComponent.hpp"
#include "Core/Structs/View.hpp"
#include "Core/Structs/FrameData.hpp"
#include <SDL3/SDL.h>
#include <iostream>

World::World(Appstate& appstate) : GameObject(), renderer(appstate) {
    
    canTick = true;
    deltaSeconds = 0;
    running.store(false);

    if (!renderer.Initialize()) {
        SDL_Log("Failed to start World Renderer");
        return;
    }
}

World::~World() {
    loadedLevels.clear();
}

Level* World::Initialize(const std::string& startLevelName) {
    Level* level = CreateInitialLevel(startLevelName);

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

void World::Tick(uint64_t deltaTime) {
    GameObject::Tick(deltaTime);
    for (Level* level : loadedLevels) {
        const std::vector<Entity*> entities = level->GetAllEntities();
        for (Entity* entity : entities) {

        }
    }
    //eventManager.ProcessEvents();
}

void World::DestroyGameObject() {
    GameObject::DestroyGameObject();
}

void World::Render() {

    FrameData frame{};
    // TODO: Need a camera view
    // frame.view = &GetCameraView();

    renderer.RenderAndSubmit(frame);
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
    renderer.RegisterMesh(mesh);
}

void World::DeregisterMesh(MeshComponent* mesh) {
    renderer.DeregisterMesh(mesh);
}

void World::RegisterSprite(SpriteComponent* sprite) {
    renderer.RegisterSprite(sprite);
}

void World::DeregisterSprite(SpriteComponent* sprite) {
    renderer.DeregisterSprite(sprite);
}
