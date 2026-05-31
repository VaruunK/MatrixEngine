#include "PhysicsComponent.hpp"
#include "Core/GameObject/Entity/Entity.hpp"
#include <iostream>
#include "Engine.hpp"

void PhysicsComponent::Initialize(Entity* compOwner, vec2f initVelocity, bool gravityEnabled, float gravity, float mass) {
    Component::Initialize(compOwner);

    if (!compOwner->canTick) {
        return;
    }

    vec2f position = compOwner->GetPosition();

    PhysicsState initialState{};
    initialState.position = position;
    initialState.velocity = initVelocity;
    initialState.gravityEnabled = gravityEnabled;
    initialState.gravity = gravity;
    initialState.mass = mass;
    initialState.active = true;

    handle = Engine::GetEngine().GetPhysicsManager().RegisterPhysicsComponent(initialState);

    cachedPosition = position;
    cachedVeclocity = initVelocity;
    cachedGravityEnabled = gravityEnabled;
    cachedGravity = gravity;
    cachedMass = mass;
}

void PhysicsComponent::Update(uint64_t deltaTime) {
    if (!handle) {
        return;
    }
    auto state = Engine::GetEngine().GetPhysicsManager().GetReadState(*handle);
    owner->SetPosition(state.value().position);
}

void PhysicsComponent::DestroyComponent() {
    Engine::GetEngine().GetPhysicsManager().DeregisterPhysicsComponent(*handle);
    handle = nullopt;
}

void PhysicsComponent::SetMass(float mass) {
    cachedMass = mass;

    if (handle) {
        Engine::GetEngine().GetPhysicsManager().SetMass(*handle, mass);
    }
}

void PhysicsComponent::EnableGravity(bool enabled) {
    cachedGravityEnabled = enabled;

    if (handle) {
        Engine::GetEngine().GetPhysicsManager().EnableGravity(*handle, enabled);
    }
}

void PhysicsComponent::SetGravity(float gravity) {
    cachedGravity = gravity;

    if (handle) {
        Engine::GetEngine().GetPhysicsManager().SetGravity(*handle, gravity);
    }
}

void PhysicsComponent::SetVelocity(vec2f velocity) {
    cachedVeclocity = velocity;

    if (handle) {
        Engine::GetEngine().GetPhysicsManager().SetVelocity(*handle, velocity);
    }
}

void PhysicsComponent::ApplyForce(vec2f force) {

    if (handle) {
        Engine::GetEngine().GetPhysicsManager().ApplyForce(*handle, force);
    }
}

void PhysicsComponent::ApplyImpulse(vec2f impulse) {
    
    if (handle) {
        Engine::GetEngine().GetPhysicsManager().ApplyImpulse(*handle, impulse);
    }
}