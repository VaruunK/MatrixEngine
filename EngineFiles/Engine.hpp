#pragma once

#include "World/World.hpp"
#include "Core/Render/Renderer.hpp"
#include "Core/ShaderManager/ShaderManager.hpp"
#include "Core/TickManager/TickManager.hpp"
#include "Core/PhysicsManager/PhysicsManager.hpp"
#include "Core/WindowManager/WindowManager.hpp"
#include "Core/AssetLoader/AssetLoader.hpp"
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
	// PhysicsManager& GetPhysicsManager() { return *physicsManager; }
	WindowManager& GetWindowManager() { return *windowManager; }
	Renderer& GetRenderer() { return *renderer; };
	SDL_GPUDevice& GetGPUDevice() { return *device; }

private:
	// need deleters for other unique pointers, probably mandatory


	struct SDLGPUDeviceDeleter {
		void operator()(SDL_GPUDevice* d) const {
			if (d) SDL_DestroyGPUDevice(d);
		}
	};

	Engine();
	
	void CreateDevice();

	std::atomic<bool> running;
	int MAX_FRAMES = 144;
	const int MAX_PHYSICS_FRAMES = 60;

	std::unique_ptr<World> world;
	std::unique_ptr<TickManager> tickManager;
	// std::unique_ptr<PhysicsManager> physicsManager;
	std::unique_ptr<WindowManager> windowManager;
	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<AssetLoader> assetLoader;

	std::unique_ptr<SDL_GPUDevice, SDLGPUDeviceDeleter> device;

	std::vector<std::thread> threads;
};