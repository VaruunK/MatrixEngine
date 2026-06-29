#include "Core/Engine.hpp"
#include <SDL3/SDL_log.h>

int main() {
    // Redirect SDL logs to a file
    FILE* logFile = fopen("engine_log.txt", "w");
    if (logFile) {
        SDL_SetLogOutputFunction([](void* userdata, int category, SDL_LogPriority priority, const char* message) {
            FILE* file = static_cast<FILE*>(userdata);
            fprintf(file, "[%d] %s\n", priority, message);
            fflush(file);
            }, logFile);
    }
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);

    int result = Engine::GetEngine().Run();
    if (logFile) {
        fclose(logFile);
    }

    return 0;
}