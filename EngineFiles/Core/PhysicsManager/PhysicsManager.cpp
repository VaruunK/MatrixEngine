#include "PhysicsManager.hpp"
#include "Engine.hpp"
#include <thread>
#include <chrono>
#include <iostream>

void PhysicsManager::Run(int targetFrames) {
    const float deltaTime = 1.0f / static_cast<float>(targetFrames);
    const auto targetFrameDuration = std::chrono::microseconds(
        static_cast<int64_t>((1.0f / targetFrames) * 1'000'000.0f)
    );

    while (Engine::GetEngine().IsRunning()) {
        auto frameStart = std::chrono::steady_clock::now();

        Step(deltaTime);

        auto frameEnd = std::chrono::steady_clock::now();
        auto elapsed = frameEnd - frameStart;

        auto sleepTime = targetFrameDuration - elapsed;

        if (sleepTime.count() > 0) {
            std::this_thread::sleep_for(sleepTime);
        }
    }
}

PhysicsHandle PhysicsManager::RegisterPhysicsComponent(const PhysicsState& initial) {
    uint32_t index;
    uint32_t generation;

    if (!physicsFreeIndices.empty()) {
        index = physicsFreeIndices.back();
        physicsFreeIndices.pop_back();

        generation = ++physicsGenerations[index];
    } else {
        index = static_cast<uint32_t>(physicsBuffers[0].size());
        generation = 0;

        physicsBuffers[0].push_back(initial);
        physicsBuffers[1].push_back(initial);
        physicsGenerations.push_back(generation);
    }

    physicsBuffers[0][index].active = true;
    physicsBuffers[1][index].active = true;

    return PhysicsHandle{ index, generation };
}

CollisionHandle PhysicsManager::RegisterCollisionComponent(const CollisionState& initial) {
    uint32_t index;
    uint32_t generation;

    if (!collisionFreeIndices.empty()) {
        index = collisionFreeIndices.back();
        collisionFreeIndices.pop_back();

        generation = ++collisionGenerations[index];
    } else {
        index = static_cast<uint32_t>(collisionBuffers[0].size());
        generation = 0;

        collisionBuffers[0].push_back(initial);
        collisionBuffers[1].push_back(initial);
        collisionGenerations.push_back(generation);
    }

    collisionBuffers[0][index].active = true;
    collisionBuffers[1][index].active = true;

    return CollisionHandle{ index, generation };
}

void PhysicsManager::DeregisterPhysicsComponent(PhysicsHandle &handle) {
    if (!ValidateHandle(handle)) return;

    physicsBuffers[0][handle.index].active = false;
    physicsBuffers[1][handle.index].active = false;

    physicsFreeIndices.push_back(handle.index);
}

void PhysicsManager::DeregisterPhysicsComponent(CollisionHandle &handle) {
    if (!ValidateHandle(handle)) return;

    collisionBuffers[0][handle.index].active = false;
    collisionBuffers[1][handle.index].active = false;

    collisionFreeIndices.push_back(handle.index);
}

void PhysicsManager::SetMass(PhysicsHandle &handle, float mass) {
    int writeIdx = commandWriteIndex.load(std::memory_order_acquire);
    commandBuffers[writeIdx].push_back(Command(Command::SET_MASS, handle, mass));
}

void PhysicsManager::SetGravity(PhysicsHandle &handle, float gravity) {
    int writeIdx = commandWriteIndex.load(std::memory_order_acquire);
    commandBuffers[writeIdx].push_back(Command(Command::SET_GRAVITY, handle, gravity));
}

void PhysicsManager::EnableGravity(PhysicsHandle &handle, bool enabled) {
    int writeIdx = commandWriteIndex.load(std::memory_order_acquire);
    commandBuffers[writeIdx].push_back(Command(Command::ENABLE_GRAVITY, handle, enabled));
}

void PhysicsManager::ApplyForce(PhysicsHandle &handle, vec2f force) {
    int writeIdx = commandWriteIndex.load(std::memory_order_acquire);
    commandBuffers[writeIdx].push_back(Command(Command::APPLY_FORCE, handle, force));
}

void PhysicsManager::ApplyImpulse(PhysicsHandle &handle, vec2f impulse) {
    int writeIdx = commandWriteIndex.load(std::memory_order_acquire);
    commandBuffers[writeIdx].push_back(Command(Command::APPLY_IMPULSE, handle, impulse));
}

void PhysicsManager::SetVelocity(PhysicsHandle &handle, vec2f velocity) {
    int writeIdx = commandWriteIndex.load(std::memory_order_acquire);
    commandBuffers[writeIdx].push_back(Command(Command::SET_VELOCITY, handle, velocity));
}

void PhysicsManager::SetPosition(PhysicsHandle &handle, vec2f position) {
    int writeIdx = commandWriteIndex.load(std::memory_order_acquire);
    commandBuffers[writeIdx].push_back(Command(Command::SET_POSITION, handle, position));
}

void PhysicsManager::SetCollisionResponse(CollisionHandle &handle, CollisionResponse response) {
    int writeIdx = commandWriteIndex.load(std::memory_order_acquire);
    commandBuffers[writeIdx].push_back(Command(Command::SET_COLLISION_RESPONSE, handle, response));
}

void PhysicsManager::Step(float deltaTime) {
    int oldRead = commandReadIndex.load(std::memory_order_acquire);
    int oldWrite = commandWriteIndex.load(std::memory_order_acquire);

    commandReadIndex.store(commandProcessIndex, std::memory_order_release);
    commandWriteIndex.store(oldRead, std::memory_order_release);
    commandProcessIndex = oldWrite;

    int physicsWriteIdx = GetPhysicsWriteIndex();
    int physicsReadIdx = physicsReadIndex.load(std::memory_order_acquire);
    
    int collisionWriteIdx = GetCollisionWriteIndex();
    int collisionReadIdx = collisionReadIndex.load(std::memory_order_acquire);

    physicsBuffers[physicsWriteIdx] = physicsBuffers[physicsReadIdx];
    collisionBuffers[collisionWriteIdx] = collisionBuffers[collisionReadIdx];

    for (const Command& cmd : commandBuffers[commandProcessIndex]) {
        if (holds_alternative<PhysicsHandle>(cmd.handle)) {
            auto& handle = get<PhysicsHandle>(cmd.handle);
            if (handle.index >= physicsBuffers[physicsWriteIdx].size()) continue;

            PhysicsState& state = physicsBuffers[physicsWriteIdx][handle.index];

            if (!state.active || physicsGenerations[handle.index] != handle.generation)
                continue;

            switch (cmd.type) {

                case Command::APPLY_FORCE: {
                    vec2f acceleration = get<vec2f>(cmd.value) / state.mass;
                    state.velocity = state.velocity + acceleration * deltaTime;
                    break;
                }

                case Command::APPLY_IMPULSE: {
                    vec2f velocityChange = get<vec2f>(cmd.value) / state.mass;
                    state.velocity = state.velocity + velocityChange;
                    break;
                }

                case Command::SET_VELOCITY:
                    state.velocity = get<vec2f>(cmd.value);
                    break;

                case Command::SET_POSITION:
                    state.position = get<vec2f>(cmd.value);
                    break;

                case Command::SET_MASS:
                    state.mass = get<float>(cmd.value);
                    break;

                case Command::SET_GRAVITY:
                    state.gravity = get<float>(cmd.value);
                    break;

                case Command::ENABLE_GRAVITY:
                    state.gravityEnabled = get<bool>(cmd.value);
                    break;
            }
        } else if (std::holds_alternative<CollisionHandle>(cmd.handle)) {
            auto& handle = get<CollisionHandle>(cmd.handle);
            if (handle.index >= collisionBuffers[collisionWriteIdx].size()) continue;

            CollisionState& state = collisionBuffers[collisionWriteIdx][handle.index];

            if (!state.active || collisionGenerations[handle.index] != handle.generation)
                continue;
            
            switch (cmd.type) {
                case Command::SET_COLLISION_RESPONSE: {
                    auto& responses = state.collisionResponses;
                    auto& value = get<CollisionResponse>(cmd.value);
                    auto& type = value.type;
                    auto& response = value.response;
                    responses.insert_or_assign(type, response);
                    break;
                }
                case Command::SET_COLLISION_BOUNDS:
                    state.bounds = get<vec2f>(cmd.value);
                    break;
            }
        }
    }

    commandBuffers[commandProcessIndex].clear();

    for (size_t i = 0; i < physicsBuffers[physicsWriteIdx].size(); ++i) {
        PhysicsState& state = physicsBuffers[physicsWriteIdx][i];

        if (!state.active) continue;

        if (state.gravityEnabled) {
            state.velocity.y += state.gravity * deltaTime;
        }

        state.position = state.position + (state.velocity * deltaTime);
    }

    for (size_t i = 0; i < collisionBuffers[collisionWriteIdx].size(); ++i) {
        CollisionState& state = collisionBuffers[collisionWriteIdx][i];

        if (!state.active) continue;


    }

    int physicsRead = physicsReadIndex.load(std::memory_order_acquire);
    physicsReadIndex.store(1 - physicsRead, std::memory_order_release);

    int collisionRead = collisionReadIndex.load(std::memory_order_acquire);
    collisionReadIndex.store(1 - collisionRead, std::memory_order_release);
}

std::optional<PhysicsState> PhysicsManager::GetReadState(PhysicsHandle &handle) {
    int idx = physicsReadIndex.load(std::memory_order_acquire);

    if (handle.index >= physicsBuffers[idx].size()) {
        return std::nullopt;
    }

    const PhysicsState& state = physicsBuffers[idx][handle.index];

    if (!state.active || physicsGenerations[handle.index] != handle.generation) {
        return std::nullopt;
    }

    return state;
}

std::optional<CollisionState> PhysicsManager::GetReadState(CollisionHandle &handle) {
    int idx = collisionReadIndex.load(std::memory_order_acquire);

    if (handle.index >= collisionBuffers[idx].size()) {
        return std::nullopt;
    }

    const CollisionState& state = collisionBuffers[idx][handle.index];

    if (!state.active || collisionGenerations[handle.index] != handle.generation) {
        return std::nullopt;
    }

    return state;
}

const int PhysicsManager::GetPhysicsWriteIndex() {
	return 1 - physicsReadIndex.load(std::memory_order_acquire);
}

const int PhysicsManager::GetCollisionWriteIndex() {
    return 1 - collisionReadIndex.load(std::memory_order_acquire);
}

const bool PhysicsManager::ValidateHandle(PhysicsHandle handle) {
    if (handle.index >= physicsGenerations.size()) {
        return false;
    }

    int readIdx = physicsReadIndex.load(std::memory_order_acquire);
    if (!physicsBuffers[readIdx][handle.index].active) {
        return false;
    }

    if (physicsGenerations[handle.index] != handle.generation) {
        return false;
    }

    return true;
}

const bool PhysicsManager::ValidateHandle(CollisionHandle handle) {
    if (handle.index >= collisionGenerations.size()) {
        return false;
    }

    int readIdx = collisionReadIndex.load(std::memory_order_acquire);
    if (!physicsBuffers[readIdx][handle.index].active) {
        return false;
    }

    if (collisionGenerations[handle.index] != handle.generation) {
        return false;
    }

    return true;
}