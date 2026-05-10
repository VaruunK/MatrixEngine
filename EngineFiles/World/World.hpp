#pragma once

#include <atomic>
#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "Level/Level.hpp"
#include "Core/GameObject/GameObject.hpp"

class TickManager;

class World : public GameObject {
public:
	World();

	Level* Initialize(const std::string& startLevelName);
	void Start() override;
	void Tick(uint64_t deltaTime) override;
	void DestroyGameObject() override;
	
	bool IsRunning() { return running.load(); }
	
	std::unordered_map<std::string, std::unique_ptr<Level>>& GetAllLevels() { return levels; }
	
	Level* GetLevel(const std::string& levelName);
	Level* GetCurrentLevel() { return currentLevel; }
	Level* CreateLevel(const std::string& levelName);

	bool LoadLevel(const std::string& levelName);
	bool LoadLevel(Level* level);

	void SwitchToLevel(std::string& levelName);
	void SwitchToLevel(Level* level);

	void SetDeltaTime(uint64_t deltaTime) { deltaSeconds = deltaTime; }
protected:
private:

	Level* CreateInitialLevel(const std::string& startLevelName);

	std::atomic<bool> running = false;
	std::atomic<bool> paused = false;

	std::unordered_map<std::string, std::unique_ptr<Level>> levels;
	Level* currentLevel = nullptr;

	int maxTicks = 60;

	uint64_t deltaSeconds;
};