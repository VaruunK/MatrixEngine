#include "Agent.hpp"
#include "Core/GameObject/Component/SpriteComponent/SpriteComponent.hpp"
#include "Core/GameObject/Component/MeshComponent/MeshComponent.hpp"
// #include "Entity/Component/PhysicsComponent/PhysicsComponent.hpp"
// #include "Entity/Component/MovementComponent/MovementComponent.hpp"

Agent::Agent(Level* level) : Entity(level) {
	canTick = true;

	/*spriteComponent = AddComponent<SpriteComponent>();
	spriteComponent->Initialize(this);*/

	/*meshComponent = AddComponent<MeshComponent>();
	meshComponent->Initialize(this);*/

	/*physicsComponent = AddComponent<PhysicsComponent>();
	physicsComponent->Initialize(this);*/

	/*movementComponent = AddComponent<MovementComponent>();
	movementComponent->Initialize(this, E_Moveable);*/
}

void Agent::Start()
{
	Entity::Start();
}

void Agent::Tick(uint64_t deltaTime)
{
	Entity::Tick(deltaTime);
}

void Agent::DestroyGameObject()
{
	Entity::DestroyGameObject();
}