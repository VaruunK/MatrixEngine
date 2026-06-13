#pragma once

#include "Core/Structs/Appstate.hpp"
#include <SDL3/SDL_gpu.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

class World;

class Game {
public:
	Game();
	Game(Appstate& appstate);
	~Game() = default;

	void Initialize(std::string& name, std::string& iconFilePath);
	
	void Start();
	World* world;


private:
	SDL_GPUDevice* CreateDevice();
	SDL_Window* CreateWindow();
	
	Appstate appstate;

	std::string iconFilePath;
	std::string name;
	std::vector<std::thread> threads;
	std::atomic<bool> running = false;
};