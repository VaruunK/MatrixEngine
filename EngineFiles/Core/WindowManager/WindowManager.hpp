#pragma once

#include <vector>
#include <memory>

class Window;
struct SDL_GPUDevice;

class WindowManager {
public:
	WindowManager(SDL_GPUDevice* device);
	~WindowManager();

	Window* CreateWindow();
	Window* GetMainWindow() { return mainWindow; }
	std::vector<Window*> GetWindows() { return windows; }
private:
	Window* mainWindow;

	std::vector<Window*> windows;
};