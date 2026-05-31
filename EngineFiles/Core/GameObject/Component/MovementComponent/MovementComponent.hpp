#pragma once

#include "Core/GameObject/Component/Component.hpp"
#include "MovementTypes.hpp"

class MovementComponent : public Component {
public:
	void Initialize(Entity* compOwner, MovementType moveType = E_Moveable);
	void Update(uint64_t deltaTime) override;
	void DestroyComponent() override;

	glm::vec3 GetVelocity() { return velocity; }
	float GetXVelocity() { return velocity.x; }
	float GetYVelocity() { return velocity.y; }
protected:
private:
	glm::vec3 CalculateVelocity(uint64_t deltaTime);

	glm::vec3 velocity = vec2f(0.0f, 0.0f, 0.0f);
	MovementType movementType;
};