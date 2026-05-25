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
	Window* GetMainWindow() { return mainWindow.get(); }
	std::vector<Window*> GetWindows() { return windows; }
private:
	std::unique_ptr<Window> mainWindow;

	std::vector<Window*> windows;
};