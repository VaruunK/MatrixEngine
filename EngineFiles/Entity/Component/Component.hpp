#pragma once
#include "../Entity.hpp"

class Entity;

class Component {
public:

	virtual void Initialize(Entity* compOwner);
	virtual void Update(uint64_t deltaTime) = 0;
	virtual void DestroyComponent() = 0;

	Entity* GetOwner() { return owner; }
protected:
	Entity* owner;
private:
};