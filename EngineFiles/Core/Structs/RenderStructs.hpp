#pragma once
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec2 uv;
};

struct DrawInfo {
    uint32_t indexCount;
    uint32_t firstIndex;
    int32_t vertexOffset;
};