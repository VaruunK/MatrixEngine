#pragma once

#include <cstdint>

using namespace std;

class Component;
class Engine;
class World;

class GameObject {
public:
	GameObject();
	virtual void Start();
	virtual void Tick(uint64_t deltaTime);
	virtual void DestroyGameObject();

	World& GetWorld();

	bool canTick;
protected:
private:
	bool hasStarted;
};
