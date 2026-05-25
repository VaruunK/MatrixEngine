#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <SDL3/SDL.h>

struct Vertex {
    glm::vec3 position;
    glm::vec2 uv;
};

struct Texture {
    SDL_GPUTexture* texture;
    SDL_Surface* data;
    std::string texturePath;
};

struct Mesh {
    Texture* texture;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};