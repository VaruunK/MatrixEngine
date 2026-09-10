#ifdef MATRIX_EDITOR

#include "ProjectLoader.hpp"
#include "Core/Game/Game.hpp"

#include <filesystem>

#ifdef _WIN32
    #include <Windows.h>
#elif defined(__linux__)
    #include <dlfcn.h>
    #include <sys/wait.h>
#endif

ProjectLoader::ProjectLoader(Appstate& appstate) : appstate(appstate) {

}

Game* ProjectLoader::LoadProject(const std::string& gamePath, const std::string& gameName) {
    std::filesystem::path projectPath = gamePath;
    std::filesystem::path buildDir = projectPath / "build";
    std::filesystem::path cacheFile = buildDir / "CMakeCache.txt";

    // A build dir configured on a different OS/generator will hard-fail.
    if (std::filesystem::exists(cacheFile)) {
        std::filesystem::remove_all(buildDir);
        std::filesystem::create_directory(buildDir);
    }

    std::filesystem::path engineRuntimeDir = std::filesystem::current_path();

    std::string configureCmd =
        "cmake -S \"" + gamePath + "\" -B \"" + buildDir.string() + "\""
        " -DMATRIX_ENGINE_INSTALL=\"" + engineRuntimeDir.string() + "\"";

    #ifndef _WIN32
        // Single-config generators (Makefiles/Ninja) need this set explicitly;
        // Windows multi-config generators (VS) ignore it, which is fine.
        configureCmd += " -DCMAKE_BUILD_TYPE=Debug";
    #endif

    int configResult = system(configureCmd.c_str());
    if (configResult != 0) {
        SDL_Log("Failed to configure game project at: %s", gamePath.c_str());
        return nullptr;
    }

    #ifdef _WIN32
        std::string buildCmd =
            "cmake --build \"" +
            buildDir.string() +
            "\" --target " +
            gameName +
            " --config Debug";
    #else
        std::string buildCmd =
            "cmake --build \"" +
            buildDir.string() +
            "\" --target " +
            gameName;
    #endif

    int buildResult = system(buildCmd.c_str());

    #ifdef _WIN32
        if (buildResult != 0) {
            SDL_Log("Failed to build game project: %s", gameName.c_str());
            return nullptr;
        }
    #else
        if (!WIFEXITED(buildResult) || WEXITSTATUS(buildResult) != 0) {
            SDL_Log("Failed to build game project: %s", gameName.c_str());
            return nullptr;
        }
    #endif

    #ifdef _WIN32
        std::filesystem::path libPath = buildDir / "bin" / (gameName + ".dll");
    #else
        std::filesystem::path libPath = buildDir / "bin" / ("lib" + gameName + ".so");
    #endif

    return LoadDll(libPath.string());
}

Game* ProjectLoader::LoadDll(const std::string& filePath) {
#ifdef _WIN32
    HMODULE dll = LoadLibraryA(filePath.c_str());

    if (!dll) {
        SDL_Log("Failed to load game DLL: %s", filePath.c_str());
        return nullptr;
    }

    loadedDll = dll;

    using CreateGameFn = Game* (*)(Appstate&);
    auto createGame = reinterpret_cast<CreateGameFn>(
        GetProcAddress(dll, "CreateGame")
    );

    if (!createGame) {
        SDL_Log("DLL missing CreateGame export");
        FreeLibrary(dll);
        return nullptr;
    }

#elif defined(__linux__)

    void* dll = dlopen(filePath.c_str(), RTLD_NOW);

    if(!dll) {
        SDL_Log("Failed to load game shared library: %s", dlerror());
        return nullptr;
    }

    loadedDll = dll;

    using CreateGameFn = Game* (*)(Appstate&);

    auto createGame = reinterpret_cast<CreateGameFn>(
        dlsym(dll, "CreateGame")
    );

    if(!createGame) {
        SDL_Log("Shared library missing CreateGame export: %s", dlerror());
        dlclose(dll);
        return nullptr;
    }

#else
    SDL_Log("Dynamic library loading is not supported on this platform");
    return nullptr;
#endif
    Game* game = createGame(appstate);
    if (!game) {
        SDL_Log("CreateGame returned null");
    
        #ifdef _WIN32
            FreeLibrary(dll);
        #elif defined(__linux__)
            dlclose(dll);
        #endif
            return nullptr;
    }

    std::string levelName = "Mainlevel";
    std::string filePath2 = "";

    game->Initialize(levelName, filePath2);

    return game;
}

#endif