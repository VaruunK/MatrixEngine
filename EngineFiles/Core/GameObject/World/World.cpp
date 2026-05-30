#include "World.hpp"
#include "Core/TickManager/TickManager.hpp"
#include "Core/GameObject/World/Level/Level.hpp"
#include <SDL3/SDL.h>
#include <iostream>

World::World() : GameObject() {
    canTick = true;
    deltaSeconds = 0;
    running.store(false);
}

Level* World::Initialize(const std::string& startLevelName) {
    Level* level = CreateInitialLevel(startLevelName);
    return level;
}

void World::Start() {
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
    for (Level* level : loadedLevels) {
        const std::vector<Entity*> entities = level->GetAllEntities();
        for (Entity* entity : entities) {
            
        }
    } 
}

void World::DestroyGameObject() {
    GameObject::DestroyGameObject();
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