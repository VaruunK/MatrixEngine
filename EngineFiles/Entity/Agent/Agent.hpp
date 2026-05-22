#pragma once

#include "Entity/Entity.hpp"

class SpriteComponent;
//class MovementComponent;
//class PhysicsComponent;

class Agent : public Entity {
public:
	Agent();

	void Start() override;
	void Tick(uint64_t deltaTime) override;
	void DestroyGameObject() override;
protected:
private:
	SpriteComponent* spriteComponent;
	/*MovementComponent* movementComponent;
	PhysicsComponent* physicsComponent;*/
};