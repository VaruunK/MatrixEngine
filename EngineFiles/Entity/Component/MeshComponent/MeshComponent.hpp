#pragma once

#include "Entity/Component/Component.hpp"
#include <vector>
#include <string>

struct Mesh;
struct Texture;
struct Vertex;

class MeshComponent : public Component {
public:
    void Initialize(Entity* compOwner) override;
    void Update(uint64_t deltaTime) override {}
    void DestroyComponent() override;

    const Mesh* GetMesh() { return mesh; }
    const std::string GetTextureFilePath() { return textureFilePath; }
    const glm::mat4 GetModelMatrix(float windowAspectRatio);

    void SetMesh(Mesh* mesh);    
    void SetTexture(Texture* texture);

private:
	Mesh* mesh;
	std::string textureFilePath;
};