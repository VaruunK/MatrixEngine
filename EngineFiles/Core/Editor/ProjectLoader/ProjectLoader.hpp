#pragma once

#ifdef MATRIX_EDITOR

#include <filesystem>
#include <string>

class Game;

struct Appstate;

class ProjectLoader {
public:
	ProjectLoader(Appstate& appstate);
	Game* LoadProject(const std::string& gamePath, const std::string& gameName);
	
private:
	Game* LoadDll(const std::string& filePath);
	Appstate& appstate;
	void* loadedDll = nullptr;
};

#endif // MATRIX_EDITOR