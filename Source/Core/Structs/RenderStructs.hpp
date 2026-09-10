#pragma once

#include "Core/MatrixAPI.hpp"
#include <glm/glm.hpp>

struct MATRIX_API Vertex {
    glm::vec3 position;
    glm::vec2 uv;
};

struct MATRIX_API DrawInfo {
    uint32_t indexCount;
    uint32_t firstIndex;
    int32_t vertexOffset;
};