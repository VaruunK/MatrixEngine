#include "World.hpp"
#include "Engine.hpp"
#include <iostream>


World::World() {
    GameObject::GameObject();
    canTick = true;
    running.store(false);
}

Level* World::Initialize(const std::string& startLevelName) {
   return CreateInitialLevel(startLevelName);
}

void World::Start() {
    currentLevel->Load();
    currentLevel->Start();
}

void World::Tick(uint64_t deltaTime) {
    Engine::GetEngine().GetTickManager().Tick(deltaTime);
}

void World::DestroyGameObject() {
    GameObject::DestroyGameObject();
}

Level* World::GetLevel(const std::string& levelName) {
    auto lvl = levels.find(levelName);
    if (lvl != levels.end()) {
        return lvl->second.get();
    }
    cout << "Couldn't find Level with name " << levelName << endl;
    return nullptr;
}

Level* World::CreateLevel(const std::string& levelName) {
    if (GetLevel(levelName)) {
        cerr << "Level " << levelName << " already exists" << endl;
        return nullptr;
    }

    auto level = make_unique<Level>();

    Level* raw = level.get();
    levels.emplace(levelName, move(level));
    return raw;
}

Level* World::CreateInitialLevel(const std::string& startLevelName) {
    auto level = make_unique<Level>();

    Level* raw = level.get();
    currentLevel = raw;
    levels.emplace(startLevelName, move(level));

    return raw;
}