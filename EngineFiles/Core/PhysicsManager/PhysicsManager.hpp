#pragma once
#include "Core/Math/vec2.h"
#include "PhysicsStructs.hpp"
#include "CollisionStructs.hpp"
#include <atomic>
#include <optional>
#include <variant>
#include <vector>

class PhysicsManager {
public:
    void Run(int targetFrames);

	PhysicsHandle RegisterPhysicsComponent(const PhysicsState& initial);
	void DeregisterPhysicsComponent(PhysicsHandle &handle);

    CollisionHandle RegisterCollisionComponent(const CollisionState& initial);
    void DeregisterPhysicsComponent(CollisionHandle &handle);

	void Step(float deltaTime);

    std::optional<PhysicsState> GetReadState(PhysicsHandle &handle);
    std::optional<CollisionState> GetReadState(CollisionHandle &handle);

    void SetMass(PhysicsHandle &handle, float mass);
    void SetGravity(PhysicsHandle &handle, float gravity);
    void EnableGravity(PhysicsHandle &handle, bool enabled);
    void SetVelocity(PhysicsHandle &handle, vec2f velocity);
    void SetPosition(PhysicsHandle &handle, vec2f position);

    void ApplyForce(PhysicsHandle &handle, vec2f force);
    void ApplyImpulse(PhysicsHandle &handle, vec2f impulse);

    void SetCollisionResponse(CollisionHandle &handle, CollisionResponse response);
protected:
private:

    struct Command {
        enum Type {
            APPLY_FORCE,
            APPLY_IMPULSE,
            SET_VELOCITY,
            SET_POSITION,
            SET_MASS,
            SET_GRAVITY,
            ENABLE_GRAVITY,
            SET_COLLISION_RESPONSE,
            SET_COLLISION_BOUNDS
        };

        Type type;

        std::variant<PhysicsHandle, CollisionHandle> handle;
        std::variant<vec2f, float, bool, CollisionResponse> value;

        Command(Type t, PhysicsHandle h, vec2f v)
            : type(t), handle(std::in_place_type<PhysicsHandle>, h),
            value(std::in_place_type<vec2f>, v) {
        }

        Command(Type t, PhysicsHandle h, float v)
            : type(t), handle(std::in_place_type<PhysicsHandle>, h),
            value(std::in_place_type<float>, v) {
        }

        Command(Type t, PhysicsHandle h, bool v)
            : type(t), handle(std::in_place_type<PhysicsHandle>, h),
            value(std::in_place_type<bool>, v) {
        }

        Command(Type t, CollisionHandle h, CollisionResponse v)
            : type(t), handle(std::in_place_type<CollisionHandle>, h),
            value(std::in_place_type<CollisionResponse>, v) {
        }

        Command(Type t, CollisionHandle h, vec2f v)
            : type(t), handle(std::in_place_type<CollisionHandle>, h),
            value(std::in_place_type<vec2f>, v) {
        }
    };

    const int GetPhysicsWriteIndex();
    const int GetCollisionWriteIndex();
    const bool ValidateHandle(PhysicsHandle handle);
    const bool ValidateHandle(CollisionHandle handle);

    std::vector<PhysicsState> physicsBuffers[2];
    std::vector<CollisionState> collisionBuffers[2];
	
    std::vector<uint32_t> physicsGenerations;
    std::vector<uint32_t> physicsFreeIndices;
    
    std::vector<uint32_t> collisionGenerations;
    std::vector<uint32_t> collisionFreeIndices;

    std::atomic<int> physicsReadIndex{ 0 };
    std::atomic<int> collisionReadIndex{ 0 };

    std::vector<Command> commandBuffers[3];
    std::atomic<int> commandWriteIndex{ 0 };
    std::atomic<int> commandReadIndex{ 1 };
    int commandProcessIndex = 2;

    bool running = false;
};