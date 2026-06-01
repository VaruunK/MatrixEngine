// Controller.cpp
#include "Controller.hpp"
#include <SDL3/SDL_keyboard.h>

Controller::Controller() {
    canTick = true;
    const bool* sdlKeyState = SDL_GetKeyboardState(&numKeys);
    keyState = std::vector<bool>(sdlKeyState, sdlKeyState + numKeys);
    previousKeyState = keyState;
    mouseButtonState = SDL_GetMouseState(nullptr, nullptr);
    previousMouseButtonState = mouseButtonState;
}

void Controller::Start() {
    GameObject::Start();
}

void Controller::Tick(uint64_t deltaTime) {
    GameObject::Tick(deltaTime);

    previousKeyState = keyState;
    previousMouseButtonState = mouseButtonState;

    const bool* sdlKeyState = SDL_GetKeyboardState(&numKeys);
    keyState = std::vector<bool>(sdlKeyState, sdlKeyState + numKeys);
    mouseButtonState = SDL_GetMouseState(nullptr, nullptr);

    keyStateChanged = (keyState != previousKeyState);

    handleInput();
}

void Controller::DestroyGameObject() {
    GameObject::DestroyGameObject();
}

bool Controller::IsKeyPressed(SDL_Scancode key) {
    if (key >= 0 && key < (int)keyState.size()) {
        return keyState[key];
    }
    return false;
}

bool Controller::IsKeyJustPressed(SDL_Scancode key) {
    if (key >= 0 && key < (int)keyState.size()) {
        return keyState[key] && !previousKeyState[key];
    }
    return false;
}

bool Controller::IsKeyReleased(SDL_Scancode key) {
    if (key >= 0 && key < (int)keyState.size()) {
        return !keyState[key] && previousKeyState[key];
    }
    return false;
}

void Controller::BindKey(SDL_Scancode key,
    std::function<void()> onPress,
    std::function<void()> onRelease) {
    if (onPress) onPressedKeyBindings[key] = onPress;
    if (onRelease) onReleaseKeyBindings[key] = onRelease;
}

bool Controller::IsMouseButtonPressed(uint8_t button) {
    return (mouseButtonState & SDL_BUTTON_MASK(button)) != 0;
}

bool Controller::IsMouseButtonJustPressed(uint8_t button) {
    return (mouseButtonState & SDL_BUTTON_MASK(button)) != 0 &&
        (previousMouseButtonState & SDL_BUTTON_MASK(button)) == 0;
}

bool Controller::IsMouseButtonReleased(uint8_t button) {
    return (mouseButtonState & SDL_BUTTON_MASK(button)) == 0 &&
        (previousMouseButtonState & SDL_BUTTON_MASK(button)) != 0;
}

void Controller::BindMouseButton(uint8_t button,
    std::function<void()> onPress,
    std::function<void()> onRelease) {
    if (button > 0 && button <= 5) {
        if (onPress) onPressedMouseButtonBindings[button] = onPress;
        if (onRelease) onReleasedMouseButtonBindings[button] = onRelease;
    }
}

void Controller::BindMouseMove(std::function<void(float, float)> callback) {
    mouseMoveCallback = callback;
}

void Controller::handleInput() {
    for (const auto& [key, action] : onPressedKeyBindings) {
        if (IsKeyPressed(key)) {
            action();
        }
    }

    for (const auto& [key, action] : onReleaseKeyBindings) {
        if (IsKeyReleased(key)) {
            action();
        }
    }

    for (const auto& [button, callback] : onPressedMouseButtonBindings) {
        if (IsMouseButtonPressed(button)) {
            callback();
        }
    }

    for (const auto& [button, callback] : onReleasedMouseButtonBindings) {
        if (IsMouseButtonReleased(button)) {
            callback();
        }
    }

    float relX, relY;
    SDL_GetRelativeMouseState(&relX, &relY);
    if (mouseMoveCallback && (relX != 0.0f || relY != 0.0f)) {
        mouseMoveCallback(relX, relY);
    }
}