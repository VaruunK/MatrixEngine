#pragma once

#include "World/World.hpp"
#include "Core/Render/Renderer.hpp"
#include "Core/ShaderManager/ShaderManager.hpp"
#include "Core/TickManager/TickManager.hpp"
#include "Core/PhysicsManager/PhysicsManager.hpp"
#include <memory>
#include <atomic>

class Engine {
public:
	static Engine& GetEngine() {
		static Engine engine;
		return engine;
	}

	int Run();
	
	Engine(Engine const&) = delete;
	void operator=(Engine const&) = delete;

	bool IsRunning() { return running.load(); };
	void SetMaxFrames(int frames) { MAX_FRAMES = frames; }

	World& GetWorld() { return *world; }
	TickManager& GetTickManager() { return *tickManager; }
	PhysicsManager& GetPhysicsManager() { return *physicsManager; }
	Renderer& GetRenderer() { return *renderer; };

protected:
private:
	Engine();
	
	atomic<bool> running;
	int MAX_FRAMES = 144;
	const int MAX_PHYSICS_FRAMES = 60;
	unique_ptr<World> world;
	unique_ptr<TickManager> tickManager;
	unique_ptr<PhysicsManager> physicsManager;
	unique_ptr<Renderer> renderer;

	vector<thread> threads;
};