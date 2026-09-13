#include "Level.hpp"
#include <iostream>

Level::Level(World* world) {
    this->world = world;
}

void Level::Load() {

}

void Level::Unload() {

}

const std::vector<Entity*>& Level::GetEntitiesByClass(const Class& entityClass) {
    static const std::vector<Entity*> empty;
    std::string typeName = entityClass.typeInfo->typeName;
    auto it = entities.find(typeName);
    if (it != entities.end()) {
        return it->second;
    }
    return empty;
}