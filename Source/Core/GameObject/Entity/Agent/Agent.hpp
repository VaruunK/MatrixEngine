#pragma once

#include "Core/MatrixAPI.hpp"
#include "Core/GameObject/Entity/Entity.hpp"

class SpriteComponent;
class MeshComponent;
//class MovementComponent;
//class PhysicsComponent;

class MATRIX_API Agent : public Entity {
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