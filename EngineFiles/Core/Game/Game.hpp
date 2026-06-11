#pragma once

#include <SDL3/SDL_gpu.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

struct SDL_GPUDevice;
class World;

class Game {
public:
	Game();
	~Game() = default;
	void Initialize(std::string& name, std::string& iconFilePath);
	void Start();
	World* world;

	SDL_GPUDevice* device;
	SDL_Window* window;
private:
	void CreateDevice();
	void CreateWindow();
	
	std::string iconFilePath;
	std::string name;
	std::vector<std::thread> threads;
	std::atomic<bool> running = false;
};