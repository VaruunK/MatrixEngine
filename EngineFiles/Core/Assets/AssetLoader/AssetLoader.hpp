#pragma once

#include <string>
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>

struct Mesh;
struct Texture;
struct SDL_GPUDevice;

class AssetLoader {
	friend class Engine;
public:
	AssetLoader();
	~AssetLoader() = default;
	Texture* CreateTexture(const std::string& textureFilePath);
	Mesh* CreateMesh(const std::string& meshFilePath);
	Mesh* CreateMesh(const std::string& meshFilePath, const std::string& textureFilePath);
private:
	void ProcessNode(aiNode* node, const aiScene* scene, Mesh* newMesh, const aiMatrix4x4& parentTransform);
	Assimp::Importer importer;

	SDL_GPUDevice* device;
};

inline AssetLoader GAssetLoader;