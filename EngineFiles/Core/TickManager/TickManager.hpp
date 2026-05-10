#pragma once

#include <vector>
#include <algorithm>

class GameObject;

using namespace std;

class TickManager {
public:

    void RegisterToTick(GameObject* gameObject);
    void DeregisterFromTick(GameObject* gameObject);

    void Tick(uint64_t deltaTime);

    const vector<GameObject*>& GetTickingGameObjects() { return tickingGameObjects; }

private:;
    vector<GameObject*> tickingGameObjects;
};
