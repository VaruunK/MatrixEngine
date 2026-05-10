#pragma once

#include "Entity/Component/Component.hpp"
#include "MovementTypes.hpp"
#include "Core/Math/vec2.h"


class MovementComponent : public Component {
public:
	void Initialize(Entity* compOwner, MovementType moveType = E_Moveable);
	void Update(uint64_t deltaTime) override;
	void DestroyComponent() override;

	vec2f GetVelocity() { return velocity; }
	float GetXVelocity() { return velocity.x; }
	float GetYVelocity() { return velocity.y; }
protected:
private:
	vec2f CalculateVelocity(uint64_t deltaTime);

	vec2f velocity = vec2f(0.0f, 0.0f);
	MovementType movementType;
};