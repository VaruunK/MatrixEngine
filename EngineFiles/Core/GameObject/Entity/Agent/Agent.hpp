#pragma once

#include "Core/GameObject/Entity/Entity.hpp"

class SpriteComponent;
class MeshComponent;
//class MovementComponent;
//class PhysicsComponent;

class Agent : public Entity {
public:
	Agent(Level* level);

	void Start() override;
	void Tick(uint64_t deltaTime) override;
	void DestroyGameObject() override;
protected:
private:
	//SpriteComponent* spriteComponent;
	//MeshComponent* meshComponent;
	/*MovementComponent* movementComponent;
	PhysicsComponent* physicsComponent;*/
};