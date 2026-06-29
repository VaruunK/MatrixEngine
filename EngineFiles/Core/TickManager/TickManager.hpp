#pragma once

// does this need the MATRIX_API does this reach across dlls? better safe than sorry for now but reevaluate eventually

#include "Core/MatrixAPI.hpp"
#include <vector>
#include <algorithm>

class GameObject;

class MATRIX_API TickManager {
public:

    void RegisterToTick(GameObject* gameObject);
    void DeregisterFromTick(GameObject* gameObject);

    void Tick(uint64_t deltaTime);

    const std::vector<GameObject*>& GetTickingGameObjects() { return tickingGameObjects; }

private:;
    std::vector<GameObject*> tickingGameObjects;
};
