#pragma once

#include <vector>
#include <algorithm>

class GameObject;

class TickManager {
public:

    void RegisterToTick(GameObject* gameObject);
    void DeregisterFromTick(GameObject* gameObject);

    void Tick(uint64_t deltaTime);

    const std::vector<GameObject*>& GetTickingGameObjects() { return tickingGameObjects; }

private:;
    std::vector<GameObject*> tickingGameObjects;
};
