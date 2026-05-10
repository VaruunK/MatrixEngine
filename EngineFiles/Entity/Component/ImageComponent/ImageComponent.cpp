#include "ImageComponent.hpp"
#include "Engine.hpp"

void ImageComponent::Initialize(Entity* compOwner) {
    Component::Initialize(compOwner);
}

void ImageComponent::Initialize(Entity* compOwner, const string& filepath) {
    Initialize(compOwner);
    textureFilePath = filepath;
    Engine::GetEngine().GetRenderer().RegisterImageComponent(this);
}

void ImageComponent::DestroyComponent()
{
    Engine::GetEngine().GetRenderer().DeregisterImageComponent(this);
}

SDL_Texture* ImageComponent::GetTexture() const {
    return Engine::GetEngine().GetRenderer().GetOrLoadTexture(textureFilePath);
}

void ImageComponent::SetTexture(const string& filepath) {
    textureFilePath = filepath;   
}