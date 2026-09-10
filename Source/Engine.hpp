#pragma once

#include "Core/Structs/Appstate.hpp"
#include <memory>
#include <atomic>
#include <vector>
#include <thread>

class Editor;
class ProjectSelector;
class ProjectLoader;
class Game;

struct SDL_Surface;

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
private:

	Engine();
	
	void Shutdown();

	SDL_GPUDevice* CreateDevice();
	SDL_Window* CreateSDLWindow();

	SDL_Surface* icon = nullptr;

	std::atomic<bool> running;
	int MAX_FRAMES = 144;
	const int MAX_PHYSICS_FRAMES = 60;
	
	ProjectSelector* projectSelector;
	ProjectLoader* projectLoader;
	Editor* editor;

	Game* game;

	Appstate appstate;

	std::vector<std::thread> threads;
};