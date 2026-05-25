cmake_minimum_required(VERSION 3.21)
project(Matrix_Engine LANGUAGES C CXX)

# -----------------------------
# Global output directories
# -----------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/lib")
set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/lib")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/lib")

# Also set the per-config versions
foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/${CONFIG_TYPE}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/${CONFIG_TYPE}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/${CONFIG_TYPE}/lib")
    set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/${CONFIG_TYPE}/lib")
    set(CMAKE_PDB_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/${CONFIG_TYPE}/lib")
endforeach()

# -----------------------------
# Executable
# -----------------------------
add_executable(Matrix
    EngineFiles/Engine.cpp
    
    EngineFiles/Core/GameObject/GameObject.cpp
    EngineFiles/Core/Render/Renderer.cpp
    EngineFiles/Core/ShaderManager/ShaderManager.cpp
    EngineFiles/Core/TickManager/TickManager.cpp
    # EngineFiles/Core/PhysicsManager/PhysicsManager.cpp
    EngineFiles/Core/WindowManager/WindowManager.cpp
    EngineFiles/Core/WindowManager/Window/Window.cpp
    EngineFiles/Entity/Entity.cpp
    #EngineFiles/Entity/Static/Static.cpp
    EngineFiles/Entity/Agent/Agent.cpp
    EngineFiles/Entity/Component/Component.cpp
    EngineFiles/Entity/Component/MeshComponent/MeshComponent.cpp
    EngineFiles/Entity/Component/SpriteComponent/SpriteComponent.cpp
    #EngineFiles/Entity/Component/PhysicsComponent/PhysicsComponent.cpp
    #EngineFiles/Entity/Component/CollisionComponent/CollisionComponent.cpp
    #EngineFiles/Entity/Component/MovementComponent/MovementComponent.cpp
    EngineFiles/World/Level/Level.cpp
    EngineFiles/World/World.cpp)

# -----------------------------
# C++ standard
# -----------------------------
target_compile_features(Matrix PRIVATE cxx_std_20)

# -----------------------------
# Include directories
# -----------------------------
target_include_directories(Matrix
    PRIVATE
        ${CMAKE_SOURCE_DIR}/EngineFiles
)

# -----------------------------
# Dependencies
# -----------------------------
add_subdirectory(vendored/SDL3 EXCLUDE_FROM_ALL)

set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_AV1  OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_JXL  OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_TIF  OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_HEIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_QOI  OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_RENDER ON CACHE BOOL "" FORCE)

add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)

set(SDLSHADERCROSS_VENDORED ON CACHE BOOL "" FORCE)
set(SDL_SHADERCROSS_SPIRV ON CACHE BOOL "" FORCE)
set(SDL_SHADERCROSS_GLSL  ON CACHE BOOL "" FORCE)
set(SDL_SHADERCROSS_HLSL  ON CACHE BOOL "" FORCE)
set(SDL_SHADERCROSS_MSL   ON CACHE BOOL "" FORCE)

add_subdirectory(vendored/SDL_shadercross EXCLUDE_FROM_ALL)

#set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
#set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)
#set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
#set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
#set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
#set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
#set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)

#add_subdirectory(vendored/Bullet EXCLUDE_FROM_ALL)

add_subdirectory(vendored/glm EXCLUDE_FROM_ALL)

target_link_libraries(Matrix
    PRIVATE
        SDL3::SDL3
        SDL3_shadercross::SDL3_shadercross
        SDL3_image::SDL3_image
        #BulletDynamics
        #BulletCollision
        #LinearMath
        glm::glm
)

# -----------------------------
# Shader Source Copy
# -----------------------------
# Find all shaders
file(GLOB_RECURSE ALL_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.hlsl")
message(STATUS "Found shaders:")

foreach(SHADER ${ALL_SHADERS})
    message(STATUS "  ${SHADER}")
endforeach()

# Copy shader sources to output directory
add_custom_target(CopyShaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:Matrix>/shaders
    COMMENT "Copying shader source files"
)
#add_dependencies(Matrix CopyShaders)

# -----------------------------
# Assets copy
# -----------------------------
add_custom_command(TARGET Matrix POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Content
        $<TARGET_FILE_DIR:Matrix>/Content
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/EngineFiles/Engine.png
        $<TARGET_FILE_DIR:Matrix>/Engine.png
    COMMENT "Copying assets"
)