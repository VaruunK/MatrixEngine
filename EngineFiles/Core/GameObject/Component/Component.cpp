#include "Component.hpp"
#include "Core/GameObject/Entity/Entity.hpp"
#include <glm/gtc/quaternion.hpp>

Component::Component(Entity* owner) : GameObject() {
    this->owner = owner;

    localTransform.location = glm::vec3(0.0f);
    localTransform.rotation = glm::vec3(0.0f);
    localTransform.scale = glm::vec3(1.0f);
    RecalculateGlobalTransform();
}

void Component::Start() {
    GameObject::Start();
}

void Component::Tick(uint64_t deltaTime) {
    GameObject::Tick(deltaTime);
}

void Component::DestroyGameObject() {
    GameObject::DestroyGameObject();
}

glm::vec3 Component::GetLocalComponentLocation() {
    return localTransform.location;
}

glm::vec3 Component::GetLocalComponentRotation() {
    return localTransform.rotation;
}

glm::vec3 Component::GetLocalComponentScale() {
    return localTransform.scale;
}

Transform Component::SetLocalComponentTransform(Transform& newTransform) {
    SetLocalComponentLocation(newTransform.location);
    SetLocalComponentRotation(newTransform.rotation);
    SetLocalComponentScale(newTransform.scale);
    return GetLocalComponentTransform();
}

glm::vec3 Component::SetLocalComponentLocation(glm::vec3& location) {
    localTransform.location = location;
    RecalculateGlobalTransform();
    return GetLocalComponentLocation();
}

glm::vec3 Component::SetLocalComponentRotation(glm::vec3& rotation) {
    auto normalize = [](float angle) {
        angle = fmod(angle, 360.0f);
        if (angle < 0.0f) angle += 360.0f;
        return angle;
        };

    localTransform.rotation.x = normalize(rotation.x);
    localTransform.rotation.y = normalize(rotation.y);
    localTransform.rotation.z = normalize(rotation.z);

    RecalculateGlobalTransform();
    return GetLocalComponentRotation();
}

glm::vec3 Component::SetLocalComponentScale(glm::vec3& scale) {
    auto validate = [](float scalar) {
        if (scalar < 0.0f) return 1.0f;
        return scalar;
        };

    localTransform.scale.x = validate(scale.x);
    localTransform.scale.y = validate(scale.y);
    localTransform.scale.z = validate(scale.z);

    RecalculateGlobalTransform();
    return GetLocalComponentScale();
}

glm::vec3 Component::GetComponentLocation() {
    return transform.location;
}

glm::vec3 Component::GetComponentRotation() {
    return transform.rotation;
}

glm::vec3 Component::GetComponentScale() {
    return transform.scale;
}

Transform Component::GetComponentTransform() {
    return transform;
}

void Component::RecalculateGlobalTransform() {
    if (!owner) {
        transform = localTransform;
        return;
    }

    Transform ownerTransform = owner->GetTransform();

    transform.rotation = ownerTransform.rotation + localTransform.rotation;
    auto normalize = [](float angle) {
        angle = fmod(angle, 360.0f);
        if (angle < 0.0f) angle += 360.0f;
        return angle;
        };
    transform.rotation.x = normalize(transform.rotation.x);
    transform.rotation.y = normalize(transform.rotation.y);
    transform.rotation.z = normalize(transform.rotation.z);

    transform.scale = ownerTransform.scale * localTransform.scale;

    glm::vec3 scaledLocal = localTransform.location * ownerTransform.scale;

    glm::quat ownerQuat = glm::quat(glm::radians(ownerTransform.rotation));
    glm::vec3 rotatedLocal = ownerQuat * scaledLocal;

    transform.location = ownerTransform.location + rotatedLocal;
}