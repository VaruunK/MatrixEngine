#pragma once

#include <string>
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>

struct Mesh;
struct Texture;

class AssetLoader {
public:
	Texture* CreateTexture(const std::string& textureFilePath);
	Mesh* CreateMesh(const std::string& meshFilePath);
	Mesh* CreateMesh(const std::string& meshFilePath, const std::string& textureFilePath);
private:
	void ProcessNode(aiNode* node, const aiScene* scene, Mesh* newMesh, const aiMatrix4x4& parentTransform);
	Assimp::Importer importer;
};