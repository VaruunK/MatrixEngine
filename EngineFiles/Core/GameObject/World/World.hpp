#pragma once

#include <atomic>
#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "Level/Level.hpp"
#include "Core/GameObject/GameObject.hpp"

class TickManager;
class WorldRenderer;
class MeshComponent;
class SpriteComponent;

struct Appstate;
struct SDL_GPUCommandBuffer;
struct SDL_GPUTexture;
struct FrameData;

class World : public GameObject {
public:
	World();

	Level* Initialize(Appstate& appstate, const std::string& startLevelName);
	void Start() override;
	void Tick(uint64_t deltaTime);
	void DestroyGameObject() override;
	
	bool IsRunning() { return running.load(); }
	
	void Render();

	std::unordered_map<std::string, Level*>& GetAllLevels() { return levels; }
	
	Level* GetLevel(const std::string& levelName);
	Level* CreateLevel(const std::string& levelName);

	bool LoadLevel(const std::string& levelName);
	bool LoadLevel(Level* level);

	void SwitchToLevel(std::string& levelName);
	void SwitchToLevel(Level* level);

	void RegisterMesh(MeshComponent* mesh);
	void DeregisterMesh(MeshComponent* mesh);

	void RegisterSprite(SpriteComponent* sprite);
	void DeregisterSprite(SpriteComponent* sprite);

	void SetDeltaTime(uint64_t deltaTime) { deltaSeconds = deltaTime; }

	WorldRenderer* GetWorldRenderer() { return renderer; }
protected:
private:

	WorldRenderer* renderer;

	Level* CreateInitialLevel(const std::string& startLevelName);

	std::atomic<bool> running = false;
	std::atomic<bool> paused = false;

	std::unordered_map<std::string, Level*> levels;
	std::vector<Level*> loadedLevels;
	Level* mainLevel;

	TickManager* tickManager;
	int maxTicks = 60;

	uint64_t deltaSeconds;
};