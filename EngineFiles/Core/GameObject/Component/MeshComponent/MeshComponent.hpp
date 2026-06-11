#pragma once

#include "Core/GameObject/Component/Component.hpp"

class Entity;

struct Mesh;
struct Texture;

class MeshComponent : public Component {
public:
    MeshComponent(Entity* owner);

    void Start() override;
    void Tick(uint64_t deltaTime) override {}
    void DestroyGameObject() override;

    const Mesh* GetMesh() { return mesh; }
    const glm::mat4 GetModelMatrix(float windowAspectRatio);

    void SetMesh(Mesh* mesh);
    void SetTexture(Texture* texture);

private:
	Mesh* mesh = nullptr;
};