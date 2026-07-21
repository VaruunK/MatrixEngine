#pragma once

#include "Core/MatrixAPI.hpp"
#include "Core/GameObject/Component/Component.hpp"
#include "MeshComponent.reflected.hpp"

class Entity;

struct Mesh;
struct Texture;

CLASS()
class MATRIX_API MeshComponent : public Component {
    REFLECTION()
public:
    MeshComponent(Entity* owner);

    FUNCTION()
    void Start() override;
    FUNCTION()
    void Tick(uint64_t deltaTime) override {}
    FUNCTION()
    void DestroyGameObject() override;

    FUNCTION()
    Mesh* GetMesh() { return mesh; }

    FUNCTION()
    glm::mat4 GetModelMatrix(float windowAspectRatio);

    FUNCTION()
    void SetMesh(Mesh* mesh);
    /*FUNCTION()
    void SetTexture(Texture* texture);*/

private:
    FIELD()
	Mesh* mesh = nullptr;
};