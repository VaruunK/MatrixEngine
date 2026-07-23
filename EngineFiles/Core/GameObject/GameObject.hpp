#pragma once

#include "Core/MatrixAPI.hpp"
#include "Core/ReflectionMacros.hpp"
#include <cstdint>
#include "GameObject.reflected.hpp"

class World;
class Level;
class ReflectedClass;

CLASS()
class MATRIX_API GameObject {
	REFLECTION()
public:
	GameObject();

	// No copy
	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;

	// No move
	GameObject(GameObject&&) = delete;
	GameObject& operator=(GameObject&&) = delete;

	FUNCTION()
	virtual void Start();
	
	FUNCTION()
	virtual void Tick(uint64_t deltaTime);
	
	FUNCTION()
	virtual void DestroyGameObject();

	FIELD()
	bool canTick;
protected:
private:

	FIELD()
	bool hasStarted;
};
