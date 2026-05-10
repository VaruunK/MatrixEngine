#pragma once

#include "Entity/Entity.hpp"

class ImageComponent;
class MovementComponent;
class PhysicsComponent;

class Agent : public Entity {
public:
	Agent();

	void Start() override;
	void Tick(uint64_t deltaTime) override;
	void DestroyGameObject() override;
protected:
private:
	ImageComponent* imageComponent;
	MovementComponent* movementComponent;
	PhysicsComponent* physicsComponent;
};