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

    EngineFiles/Core/Assets/AssetLoader/AssetLoader.cpp
    EngineFiles/Core/Assets/DefaultAssets/DefaultAssets.cpp

    EngineFiles/Core/Viewport/Viewport.cpp
    EngineFiles/Core/Viewport/ViewportRenderer/ViewportRenderer.cpp
    EngineFiles/Core/Viewport/ViewportController/ViewportController.cpp
    EngineFiles/Core/Viewport/ViewportCamera/ViewportCamera.cpp
    
    EngineFiles/Core/ShaderManager/ShaderManager.cpp
    EngineFiles/Core/TickManager/TickManager.cpp
    
    # EngineFiles/Core/PhysicsManager/PhysicsManager.cpp
    EngineFiles/Core/WindowManager/WindowManager.cpp
    EngineFiles/Core/WindowManager/Window/Window.cpp

    EngineFiles/Core/GameObject/GameObject.cpp
    EngineFiles/Core/GameObject/Component/Component.cpp
    EngineFiles/Core/GameObject/Component/MeshComponent/MeshComponent.cpp
    EngineFiles/Core/GameObject/Component/SpriteComponent/SpriteComponent.cpp

    EngineFiles/Core/GameObject/Entity/Entity.cpp
    #EngineFiles/Entity/Static/Static.cpp
    EngineFiles/Core/GameObject/Entity/Agent/Agent.cpp
    #EngineFiles/Entity/Component/PhysicsComponent/PhysicsComponent.cpp
    #EngineFiles/Entity/Component/CollisionComponent/CollisionComponent.cpp
    #EngineFiles/Entity/Component/MovementComponent/MovementComponent.cpp
    EngineFiles/Core/GameObject/Controller/Controller.cpp
    
    EngineFiles/Core/GameObject/World/World.cpp
    EngineFiles/Core/GameObject/World/Level/Level.cpp
    EngineFiles/Core/GameObject/World/WorldRenderer/WorldRenderer.cpp)


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

# --- SDL3 ---
add_subdirectory(vendored/SDL3 EXCLUDE_FROM_ALL)

## --- Build zlib once into the EXACT directory SDL_image would use ---
set(ZLIB_SOURCE_DIR "${CMAKE_SOURCE_DIR}/vendored/SDL_image/external/zlib")
set(ZLIB_BINARY_DIR "${CMAKE_BINARY_DIR}/vendored/SDL_image/external/zlib-build")

add_subdirectory("${ZLIB_SOURCE_DIR}" "${ZLIB_BINARY_DIR}" EXCLUDE_FROM_ALL)

if(NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()

# --- SDL_image ---
set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_AV1 OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_JXL OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_TIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_HEIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_QOI OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_RENDER ON  CACHE BOOL "" FORCE)
set(SDLIMAGE_ZLIB OFF CACHE BOOL "" FORCE)

set(ZLIB_FOUND TRUE CACHE BOOL "" FORCE)
set(ZLIB_LIBRARY zlibstatic CACHE STRING "" FORCE)
set(ZLIB_INCLUDE_DIR "${ZLIB_SOURCE_DIR};${ZLIB_BINARY_DIR}" CACHE PATH "" FORCE)

# Patch SDL_image's CMakeLists to guard its zlib add_subdirectory
set(SDLIMAGE_CMAKE "${CMAKE_SOURCE_DIR}/vendored/SDL_image/CMakeLists.txt")
file(READ "${SDLIMAGE_CMAKE}" SDLIMAGE_CMAKE_CONTENT)
if(NOT SDLIMAGE_CMAKE_CONTENT MATCHES "if\\(NOT TARGET zlibstatic\\)")
    string(REPLACE
        "add_subdirectory(external/zlib external/zlib-build EXCLUDE_FROM_ALL)"
        "if(NOT TARGET zlibstatic)\n        add_subdirectory(external/zlib external/zlib-build EXCLUDE_FROM_ALL)\n    endif()"
        SDLIMAGE_CMAKE_CONTENT "${SDLIMAGE_CMAKE_CONTENT}"
    )
    file(WRITE "${SDLIMAGE_CMAKE}" "${SDLIMAGE_CMAKE_CONTENT}")
    message(STATUS "Patched SDL_image CMakeLists.txt to guard zlib add_subdirectory")
endif()

add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)

# Fix bad INTERFACE_INCLUDE_DIRECTORIES set by SDL_image's zlib AFTER it runs
set_target_properties(zlibstatic PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES
        "$<BUILD_INTERFACE:${ZLIB_SOURCE_DIR}>;$<BUILD_INTERFACE:${ZLIB_BINARY_DIR}>"
)

# --- assimp ---
set(ASSIMP_BUILD_ZLIB OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Importers
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_COLLADA_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)

# Exporters
set(ASSIMP_BUILD_OBJ_EXPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_EXPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_COLLADA_EXPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_EXPORTER ON CACHE BOOL "" FORCE)

add_subdirectory(vendored/assimp EXCLUDE_FROM_ALL)

target_link_libraries(assimp PRIVATE zlibstatic)

# --- SDL_shadercross ---
set(SDLSHADERCROSS_VENDORED ON CACHE BOOL "" FORCE)
set(SDL_SHADERCROSS_SPIRV ON CACHE BOOL "" FORCE)
set(SDL_SHADERCROSS_GLSL ON CACHE BOOL "" FORCE)
set(SDL_SHADERCROSS_HLSL ON CACHE BOOL "" FORCE)
set(SDL_SHADERCROSS_MSL ON CACHE BOOL "" FORCE)

add_subdirectory(vendored/SDL_shadercross EXCLUDE_FROM_ALL)

# --- GLM ---
add_subdirectory(vendored/glm EXCLUDE_FROM_ALL)

# --- IMGUI ---
add_subdirectory(vendored/imgui EXCLUDE_FROM_ALL)

# -----------------------------
# Link libraries
# -----------------------------
target_link_libraries(Matrix
    PRIVATE
        SDL3::SDL3
        SDL3_shadercross::SDL3_shadercross
        SDL3_image::SDL3_image
        #BulletDynamics
        #BulletCollision
        #LinearMath
        glm::glm
        assimp::assimp
        imgui::imgui
)

# -----------------------------
# Shader source copy
# -----------------------------
file(GLOB_RECURSE ALL_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.hlsl")
message(STATUS "Found shaders:")

foreach(SHADER ${ALL_SHADERS})
    message(STATUS "  ${SHADER}")
endforeach()

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