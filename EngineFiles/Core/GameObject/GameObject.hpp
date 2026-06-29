#pragma once

#include "Core/MatrixAPI.hpp"
#include <cstdint>

class World;
class Level;

class MATRIX_API GameObject {
public:
	GameObject();

	// No copy
	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;

	// No move
	GameObject(GameObject&&) = delete;
	GameObject& operator=(GameObject&&) = delete;

	virtual void Start();
	virtual void Tick(uint64_t deltaTime);
	virtual void DestroyGameObject();

	bool canTick;
protected:
private:
	bool hasStarted;
};
