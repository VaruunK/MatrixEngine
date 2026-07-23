#pragma once

#include "Core/MatrixAPI.hpp"
#include "Core/GameObject/GameObject.hpp"
#include "Core/GameObject/World/WorldRenderer/WorldRenderer.hpp"
#include "Level/Level.hpp"
#include <atomic>
#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "World.reflected.hpp"

class TickManager;
class WorldRenderer;
class MeshComponent;
class SpriteComponent;

struct Appstate;
struct SDL_GPUCommandBuffer;
struct SDL_GPUTexture;
struct FrameData;

CLASS()
class MATRIX_API World : public GameObject {

	REFLECTION()

public:
	World(Appstate& appstate);
	~World();

	// No copy
	World(const World&) = delete;
	World& operator=(const World&) = delete;

	// No move
	World(World&&) = delete;
	World& operator=(World&&) = delete;

	Level* Initialize(const std::string& startLevelName);
	
	FUNCTION()
	void Start() override;
	
	FUNCTION()
	void Tick(uint64_t deltaTime);
	
	FUNCTION()
	void DestroyGameObject() override;
	
	FUNCTION()
	bool IsRunning() { return running.load(); }
	
	void Render();

	FUNCTION()
	Level* GetLevel(const std::string& levelName);
	
	Level* CreateLevel(const std::string& levelName);

	// FUNCTION()
	bool LoadLevel(const std::string& levelName);
	
	// FUNCTION()
	bool LoadLevel(Level* level);

	// FUNCTION()
	void SwitchToLevel(std::string& levelName);
	
	// FUNCTION()
	void SwitchToLevel(Level* level);

	void SetDeltaTime(uint64_t deltaTime) { deltaSeconds = deltaTime; }

	FIELD()
	std::unordered_map<std::string, Level*> levels;

	void RegisterMesh(MeshComponent* mesh);
	void DeregisterMesh(MeshComponent* mesh);

	void RegisterSprite(SpriteComponent* sprite);
	void DeregisterSprite(SpriteComponent* sprite);

	WorldRenderer& GetWorldRenderer() { return renderer; }
protected:
private:

	WorldRenderer renderer;

	Level* CreateInitialLevel(const std::string& startLevelName);

	std::atomic<bool> running = false;
	std::atomic<bool> paused = false;


	std::vector<Level*> loadedLevels;
	Level* mainLevel = nullptr;

	TickManager* tickManager;
	int maxTicks = 60;

	uint64_t deltaSeconds;
};