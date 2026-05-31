#include "SpriteComponent.hpp"
#include "Core/Structs/RenderStructs.hpp"
#include "Core/Structs/AssetStructs.hpp"
#include "Engine.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>
#include <glm/ext/matrix_transform.hpp>
// #include <iostream>

void SpriteComponent::Initialize(Entity* compOwner) {
    Component::Initialize(compOwner);
}

const glm::mat4 SpriteComponent::GetModelMatrix(float windowAspectRatio) {
    
    float imageAspectRatio = static_cast<float>(texture->data->w) /
        static_cast<float>(texture->data->h);
    glm::vec3 scale = glm::vec3(GetComponentScale() * (imageAspectRatio / windowAspectRatio));

    glm::mat4 model = glm::mat4(1.0f);
    
    model = glm::translate(model, GetComponentLocation());

    glm::vec3 rotation = GetComponentRotation();
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));  // pitch
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));  // yaw
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));  // roll

    // std::cout << "X: " << GetComponentRotation().x << " Y: " << GetComponentRotation().y << " Z: " << GetComponentRotation().z << std::endl;
    
    model = glm::scale(model, scale);

    return model;
}

void SpriteComponent::SetTexture(Texture* texture) {
    Engine::GetEngine().GetWorld().DeregisterSprite(this);

    if (this->texture) {
        free(this->texture);
    }

    this->texture = texture;

    Engine::GetEngine().GetWorld().RegisterSprite(this);
}

void SpriteComponent::DestroyComponent() {
    Engine::GetEngine().GetWorld().DeregisterSprite(this);
}