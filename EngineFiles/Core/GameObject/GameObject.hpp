#pragma once
#include <cstdint>
#include <memory>

class World;
class Level;

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
	Level* currentLevel;
};
