#include "ProjectLoader.hpp"
#include "Core/Game/Game.hpp"
#include <Windows.h>

ProjectLoader::ProjectLoader(Appstate& appstate) : appstate(appstate) {

}

Game* ProjectLoader::LoadProject(const std::string& gamePath, const std::string& gameName) {
    std::string buildDir = gamePath + "\\build";

    std::string configureCmd = "cmake -S \"" + gamePath + "\" -B \"" + buildDir + "\"";
    int configResult = system(configureCmd.c_str());
    if (configResult != 0) {
        SDL_Log("Failed to configure game project at: %s", gamePath.c_str());
        return nullptr;
    }

    std::string buildCmd = "cmake --build \"" + buildDir + "\" --target " + gameName + " --config Debug";
    int buildResult = system(buildCmd.c_str());
    if (buildResult != 0) {
        SDL_Log("Failed to build game project at: %s", gamePath.c_str());
        return nullptr;
    }

    return LoadDll(buildDir + "\\Debug\\" + gameName + ".dll");
}

Game* ProjectLoader::LoadDll(const std::string& filePath) {
    HMODULE dll = LoadLibraryA(filePath.c_str());
    if (!dll) {
        SDL_Log("Failed to load game DLL: %s", filePath.c_str());
        return nullptr;
    }

    loadedDll = dll;

    using CreateGameFn = Game * (*)(Appstate&);
    auto createGame = (CreateGameFn)GetProcAddress(dll, "CreateGame");

    if (!createGame) {
        SDL_Log("DLL missing CreateGame export");
        FreeLibrary(dll);
        return nullptr;
    }

    Game* game = createGame(appstate);
    if (!game) {
        SDL_Log("CreateGame returned null");
        return nullptr;
    }

    std::string levelName = "Mainlevel";
    std::string filePath2 = "";
    game->Initialize(levelName, filePath2);

    return game;
}