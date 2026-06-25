#pragma once
#include <imgui_impl_sdlgpu3.h>
#include <string>
#include <pugixml.hpp>
#include <filesystem>
#include <unordered_map>

class Game;

struct Appstate;
struct ImGuiIO;

class ProjectSelector {
	friend class Engine;
public:
	ProjectSelector(Appstate& appstate);
	~ProjectSelector();
	void Run();
private:
	void GetGameDirectory();
	void SelectGameDirectory();
	
	void Render();
	void RenderGameDirectory();
	void RenderGameButton(const std::filesystem::directory_entry& entry);
	void RenderOptionsButtons();
	void RenderCreateGameModel();

	void CreateNewGameFiles(std::string& newGamePath, char* newGameName);

	pugi::xml_document engineConfigDocument;
	pugi::xml_node gameDirectory;

	std::string configFilePath = "Matrix.config";
	std::string gameDirectoryString;

	std::string selectedGamePathString = "";

	std::unordered_map<std::filesystem::path, ImTextureID> gameIcons;

	ImGuiIO* io = nullptr;
	bool running = false;
	int MAX_FRAMES = 144;

	Appstate& appstate;
};