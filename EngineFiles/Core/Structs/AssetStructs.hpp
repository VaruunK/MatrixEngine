#pragma once

#include "Core/MatrixAPI.hpp"
#include "Core/Structs/RenderStructs.hpp"
#include <SDL3/SDL.h>
#include <filesystem>
#include <string>
#include <vector>

struct MATRIX_API Texture {
    SDL_GPUTexture* texture = nullptr;
    SDL_Surface* data = nullptr;
    std::filesystem::path filePath;
};

struct MATRIX_API Material {
    Texture* baseColor = nullptr;
    Texture* albedo = nullptr;
    Texture* normal = nullptr;
    Texture* orm = nullptr;
    Texture* emissive = nullptr;

    float metallic = 0.0f;
    float roughness = 1.0f;
};

struct MATRIX_API Mesh {
    Material* material = nullptr;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::filesystem::path filePath;
};