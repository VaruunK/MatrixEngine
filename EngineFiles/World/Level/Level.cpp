#include "Level.hpp"
#include <iostream>

Level::Level() {

}

void Level::Load() {
    for (auto& e : entities) {
        e->Start();
    }
}

void Level::Unload()
{
}

void Level::Start() {
    GameObject::Start();
}

void Level::Tick(uint64_t deltaTime) {
    GameObject::Tick(deltaTime);
}

void Level::DestroyGameObject()
{
    GameObject::DestroyGameObject();
}

void Level::Tick(uint64_t deltaTime, const vector<GameObject*>& tickingGameObjects) {
    Tick(deltaTime);
    for (auto& go : tickingGameObjects) {
        go->Tick(deltaTime);
    }
}