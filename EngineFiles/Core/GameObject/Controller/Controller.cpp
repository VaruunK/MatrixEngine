#include "Controller.hpp"
#include <SDL3/SDL_keyboard.h>

Controller::Controller() {
    canTick = true;
    state = std::vector<bool>(SDL_GetKeyboardState(&numKeys), SDL_GetKeyboardState(&numKeys) + numKeys);
    previousState = std::vector<bool>(SDL_GetKeyboardState(&numKeys), SDL_GetKeyboardState(&numKeys) + numKeys);
}

void Controller::Start() {
    GameObject::Start();
}

void Controller::Tick(uint64_t deltaTime) {
    GameObject::Tick(deltaTime);
    handleInput();
    stateChanged = (state != previousState);
    if (stateChanged) {
        previousState = state;
    }
    state = std::vector<bool>(SDL_GetKeyboardState(&numKeys), SDL_GetKeyboardState(&numKeys) + numKeys);
}

void Controller::DestroyGameObject() {
    GameObject::DestroyGameObject();
}

bool Controller::IsKeyPressed(SDL_Scancode PressedKey) {
    if (PressedKey >= 0 && PressedKey < state.size()) {
        return state[PressedKey];
    }
    return false;
}


bool Controller::IsKeyReleased(SDL_Scancode PressedKey) {
    if (PressedKey >= 0 && PressedKey < state.size()) {
        return !state[PressedKey];
    }
    return true;
}

void Controller::BindKey(SDL_Scancode key,
    std::function<void()> onPress,
    std::function<void()> onRelease) {
    onPressedBindings[key] = onPress;
    if (onRelease) {
        onReleaseBindings[key] = onRelease;
    }
}

void Controller::handleInput() {
    
    // Check all bound keys for press events
    for (const auto& [key, action] : onPressedBindings) {
        /*if (IsKeyPressed(key) && !previousState[key]) {
            action();
        }*/
        if (IsKeyPressed(key)) {
            action();
        }
    }

    // Check all bound keys for release events
    for (const auto& [key, action] : onReleaseBindings) {
        if (IsKeyReleased(key) && previousState[key]) {
            action();
        }
    }
}