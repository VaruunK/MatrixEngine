#pragma once
#include "Core/Editor/ContentBrowser/ContentBrowserController/ContentBrowserController.hpp"
#include <imgui.h>
#include <filesystem>

class ContentBrowser {
public:
	ContentBrowser();
	void Render(bool* active);
	void Tick(float deltaTime);

	void OpenLockPopup();
	void CloseLockPopup();
private:
	void RenderLockPopup();
	void RenderContentFolder(const std::filesystem::path& path);
	
	ContentBrowserController controller;

	bool wasFocused = false;
	
	bool locked = false;
	bool unlocked = true;
	bool lockPopup = false;

};