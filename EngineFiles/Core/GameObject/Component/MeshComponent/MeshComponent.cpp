#include "MeshComponent.hpp"
#include "Core/GameObject/World/World.hpp"
#include <glm/ext/matrix_transform.hpp>
// #include <iostream>

MeshComponent::MeshComponent(Entity* owner) : Component(owner) {
    //owner->GetLevel()->GetWorld()->RegisterMesh(this);
}

void MeshComponent::Start() {
    Component::Start();
}

void MeshComponent::DestroyGameObject() {
    owner->GetLevel()->GetWorld()->DeregisterMesh(this);
}

const glm::mat4 MeshComponent::GetModelMatrix(float windowAspectRatio) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, GetComponentLocation());

    glm::vec3 rotation = GetComponentRotation();
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));

    model = glm::scale(model, GetComponentScale());
    return model;
}

void MeshComponent::SetMesh(Mesh* mesh) {
    if (!mesh) return;

    if (this->mesh) {
        owner->GetLevel()->GetWorld()->DeregisterMesh(this);
    }
    
    this->mesh = mesh;

    owner->GetLevel()->GetWorld()->RegisterMesh(this);
}

//void MeshComponent::SetTexture(const std::string& texturePath) {
//    Engine::GetEngine().GetRenderer().DeregisterMesh(this);
//
//    textureFilePath = texturePath;
//    mesh->texture = CreateTexture(textureFilePath);
//    if (!mesh->texture) {
//        SDL_Log("Failed to create texture for %s", texturePath.c_str());
//    }
//
//    Engine::GetEngine().GetRenderer().RegisterMesh(this);
//}