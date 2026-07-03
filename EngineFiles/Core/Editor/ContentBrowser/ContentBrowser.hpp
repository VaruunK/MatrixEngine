#pragma once
#include "Core/Editor/ContentBrowser/ContentBrowserController/ContentBrowserController.hpp"
#include <filesystem>

struct Appstate;

class ContentBrowser {
public:
	ContentBrowser(Appstate& appstate);
	void Render(bool* active);
	void Tick(float deltaTime);
private:
	enum AssetType {
		MESH,
		TEXTURE,
		MATERIAL,
		NONE
	};

	void ImportNewAsset();

	void RenderBreadcrumb();
	void RenderContentFolder(const std::filesystem::path& path);
	
	void RenderRightClickPopup();
	void RenderImportMeshModel();
	void RenderDeleteFileModel();

	void RenderRenaming(const std::filesystem::path& path, std::string& name);
	void SetRenamePath();
	void SetDeletePath();

	std::filesystem::path importFile;
	std::filesystem::path renamingFile;
	std::filesystem::path deleteFile;
	
	AssetType importAssetType = NONE;

	std::vector<std::filesystem::path> history;
	size_t historyIndex = 0;
	std::filesystem::path currentPath;

	std::filesystem::path hoveredItem;   // item under the mouse this frame
	std::filesystem::path selectedItem;  // item the user clicked

	ContentBrowserController controller;

	Appstate& appstate;

	char importAssetName[64]{};
	char renameBuffer[64]{};

	bool renameFocusRequested = false;

	bool locked = false;
	bool unlocked = true;

	bool f2RenamePending = false;
	bool wasFocused = false;

	bool popup = false;
	bool deleteFilePopup = false;
};