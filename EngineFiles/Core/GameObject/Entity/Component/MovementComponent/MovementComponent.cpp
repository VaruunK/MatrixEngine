#include "MovementComponent.hpp"

void MovementComponent::Initialize(Entity* compOwner, MovementType moveType) {
	Component::Initialize(compOwner);
	movementType = moveType;
}

void MovementComponent::Update(uint64_t deltaTime) {

}

void MovementComponent::DestroyComponent() {

}

vec2f MovementComponent::CalculateVelocity(uint64_t deltaTime) {
	 vec2f velocity = owner->GetPosition() / deltaTime;
	 return velocity;
}