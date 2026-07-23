#pragma once

#include "Core/MatrixAPI.hpp"
#include "Core/Structs/RenderStructs.hpp"
#include "Core/ReflectionMacros.hpp"
#include <SDL3/SDL.h>
#include <filesystem>
#include <vector>
#include "Texture.reflected.hpp"
#include "Material.reflected.hpp"
#include "Mesh.reflected.hpp"

STRUCT()
struct MATRIX_API Texture {
    
    REFLECT_STRUCT()

    SDL_GPUTexture* texture = nullptr;
    SDL_Surface* data = nullptr;
    std::filesystem::path filePath;
};

STRUCT()
struct MATRIX_API Material {
    
    REFLECT_STRUCT()

    Texture* baseColor = nullptr;
    Texture* albedo = nullptr;
    Texture* normal = nullptr;
    Texture* orm = nullptr;
    Texture* emissive = nullptr;

    float metallic = 0.0f;
    float roughness = 1.0f;
};

STRUCT()
struct MATRIX_API Mesh {

    REFLECT_STRUCT()

    Material* material = nullptr;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::filesystem::path filePath;
};