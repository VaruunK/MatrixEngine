#pragma once
#include <imgui.h>
#include <filesystem>

class ContentBrowser {
public:
	ContentBrowser();
	void Render(bool* active);
private:

	ImGuiWindowFlags contentBarFlags;

	void RenderContentFolder(const std::filesystem::path& path);
	bool wasFocused = false;
};