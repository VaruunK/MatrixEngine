#pragma once

#include "Core/GameObject/Component/Component.hpp"
#include "Core/Structs/PhysicsStructs.hpp"
#include <optional>

class PhysicsComponent : public Component {
public:
	void Initialize(Entity* compOwner, vec2f initVelocity = vec2f(0.0f, 0.0f), bool gravityEnabled = true, float gravity = 9.8f, float mass = 1.0f);
	void Update(uint64_t deltaTime) override;
	void DestroyComponent() override;

	void SetMass(float mass);
	void SetGravity(float gravity);
	void SetVelocity(vec2f velocity);
	void ApplyForce(vec2f force);
	void ApplyImpulse(vec2f impulse);
	void EnableGravity(bool enabled);

	float GetMass() const { return cachedMass; }
	float GetGravity() const { return cachedGravity; }
	bool IsGravityEnabled() const { return cachedGravityEnabled; }
protected:
private:
	vec2f cachedPosition = vec2f(0.0f, 0.0f);
	vec2f cachedVeclocity = vec2f(0.0f, 0.0f);
	float cachedMass = 0.0f;
	float cachedGravity = 0.0f;
	bool cachedGravityEnabled = true;

	optional<PhysicsHandle> handle;
};