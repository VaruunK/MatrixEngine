#pragma once
#include "Core/Editor/ContentBrowser/ContentBrowserController/ContentBrowserController.hpp"
#include <filesystem>

struct Appstate;

class ContentBrowser {
public:
	ContentBrowser(Appstate& appstate);
	void Render(bool* active);
	void Tick(float deltaTime);

	void OpenLockPopup();
	void CloseLockPopup();
private:
	enum AssetType {
		MESH, 
		TEXTURE,
		MATERIAL,
		NONE
	};

	void RenderLockPopup();
	void RenderContentFolder(const std::filesystem::path& path);
	
	ContentBrowserController controller;

	Appstate& appstate;

	bool wasFocused = false;
	
	bool locked = false;
	bool unlocked = true;
	bool lockPopup = false;

};