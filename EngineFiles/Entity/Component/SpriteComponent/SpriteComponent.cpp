#include "SpriteComponent.hpp"
#include "Engine.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>

void SpriteComponent::Initialize(Entity* compOwner) {
    Component::Initialize(compOwner);
}

Texture* SpriteComponent::CreateTexture(const std::string& texturePath) {
    SDL_Surface* imageData = SDL_LoadSurface(texturePath.c_str());

    if (!imageData) {
        SDL_Log("Failed to load image data: %s", SDL_GetError());
        return nullptr;
    }

    SDL_Surface* converted = SDL_ConvertSurface(imageData, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(imageData);
    imageData = converted;

    SDL_GPUTextureCreateInfo textureCreateInfo{
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
        .width = static_cast<Uint32>(imageData->w),
        .height = static_cast<Uint32>(imageData->h),
        .layer_count_or_depth = 1,
        .num_levels = 1,
    };

    SDL_GPUTexture* texture = SDL_CreateGPUTexture(&Engine::GetEngine().GetGPUDevice(), &textureCreateInfo);
    if (!texture) {
        SDL_Log("Failed to create Texture");
        return nullptr;
    }
    SDL_SetGPUTextureName(&Engine::GetEngine().GetGPUDevice(), texture, texturePath.c_str());

    Texture* newTexture = new Texture;
    newTexture->texture = texture;
    newTexture->data = imageData;

    return newTexture;
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

    std::cout << "X: " << GetComponentRotation().x << " Y: " << GetComponentRotation().y << " Z: " << GetComponentRotation().z << std::endl;
    
    model = glm::scale(model, scale);

    return model;
}

void SpriteComponent::DestroyComponent() {
    Engine::GetEngine().GetRenderer().DeregisterSprite(this);
}

void SpriteComponent::SetTexture(const std::string& texturePath) {
    Engine::GetEngine().GetRenderer().DeregisterSprite(this);
    
    textureFilePath = texturePath;
    texture = CreateTexture(textureFilePath);
    if (!texture) {
        SDL_Log("Failed to create texture for %s", texturePath.c_str());
    }

    Engine::GetEngine().GetRenderer().RegisterSprite(this);
}