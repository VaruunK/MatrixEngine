cmake_minimum_required(VERSION 3.31)
project(Matrix_Engine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

# Single-config generators (Makefiles/Ninja) need an explicit build type,
# unlike multi-config generators (Visual Studio) which always have one selected.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# -----------------------------
# Global output directories
# -----------------------------
set(MATRIX_ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/lib")
set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/lib")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/lib")
set(REFLECTOR_BUILD_DIR "${CMAKE_BINARY_DIR}/reflector")
set(REFLECTOR_SOURCE_DIR "${MATRIX_ENGINE_ROOT}/Source/Reflection")

foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/${CONFIG_TYPE}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/${CONFIG_TYPE}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/${CONFIG_TYPE}/lib")
    set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/${CONFIG_TYPE}/lib")
    set(CMAKE_PDB_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/${CONFIG_TYPE}/lib")
endforeach()

message(STATUS "Configuring Reflector tool...")
execute_process(
    COMMAND ${CMAKE_COMMAND} -S "${REFLECTOR_SOURCE_DIR}" -B "${REFLECTOR_BUILD_DIR}"
    RESULT_VARIABLE REFLECTOR_CONFIGURE_RESULT
)
if(NOT REFLECTOR_CONFIGURE_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to configure Reflector tool")
endif()

message(STATUS "Building Reflector tool...")

execute_process(
    COMMAND ${CMAKE_COMMAND} --build "${REFLECTOR_BUILD_DIR}"
    RESULT_VARIABLE REFLECTOR_BUILD_RESULT
)

if(NOT REFLECTOR_BUILD_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to build Reflector tool")
endif()

find_program(REFLECTOR_EXECUTABLE
    NAMES Reflector
    PATHS "${REFLECTOR_BUILD_DIR}"
    NO_DEFAULT_PATH
)

if(NOT REFLECTOR_EXECUTABLE)
    message(FATAL_ERROR "Could not find built Reflector executable")
endif()

set(REFLECTED_HEADERS
    # Game required headers
    "${MATRIX_ENGINE_ROOT}/Source/Core/GameObject/GameObject.hpp"
    "${MATRIX_ENGINE_ROOT}/Source/Core/GameObject/World/World.hpp"
    "${MATRIX_ENGINE_ROOT}/Source/Core/GameObject/World/Level/Level.hpp"
    "${MATRIX_ENGINE_ROOT}/Source/Core/GameObject/Controller/Controller.hpp"
    "${MATRIX_ENGINE_ROOT}/Source/Core/GameObject/Entity/Entity.hpp"

    # Components
    "${MATRIX_ENGINE_ROOT}/Source/Core/GameObject/Component/Component.hpp"
    "${MATRIX_ENGINE_ROOT}/Source/Core/GameObject/Component/MeshComponent/MeshComponent.hpp"
    "${MATRIX_ENGINE_ROOT}/Source/Core/GameObject/Component/SpriteComponent/SpriteComponent.hpp"

    # Structs
    "${MATRIX_ENGINE_ROOT}/Source/Core/Structs/Transform.hpp"
    "${MATRIX_ENGINE_ROOT}/Source/Core/Structs/View.hpp"
    "${MATRIX_ENGINE_ROOT}/Source/Core/Structs/AssetStructs.hpp"
)

message(STATUS "Running Reflector over engine headers...")
execute_process(
    COMMAND "${REFLECTOR_EXECUTABLE}" "${CMAKE_BINARY_DIR}/.reflected" ${REFLECTED_HEADERS}
    RESULT_VARIABLE REFLECTOR_RUN_RESULT
)
if(NOT REFLECTOR_RUN_RESULT EQUAL 0)
    message(FATAL_ERROR "Reflector tool failed to run")
endif()

file(GLOB REFLECTED_SOURCES "${CMAKE_BINARY_DIR}/.reflected/*.reflected.cpp")
message(STATUS "Found reflected sources:")
foreach(REFLECTED ${REFLECTED_SOURCES})
    message(STATUS "  ${REFLECTED}")
endforeach()

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${REFLECTED_HEADERS})

# -----------------------------
# Engine LIBRARY
# -----------------------------
add_library(MatrixEngineLib SHARED
    # Core engine runtime
    "Source/Core/Game/Game.cpp"

    # Rendering
    "Source/Core/ShaderManager/ShaderManager.cpp"
    "Source/Core/GameObject/World/WorldRenderer/WorldRenderer.cpp"

    # GameObject system
    "Source/Core/GameObject/GameObject.cpp"
    "Source/Core/GameObject/Component/Component.cpp"
    "Source/Core/GameObject/Component/MeshComponent/MeshComponent.cpp"
    "Source/Core/GameObject/Component/SpriteComponent/SpriteComponent.cpp"
    "Source/Core/GameObject/Entity/Entity.cpp"
    "Source/Core/GameObject/Entity/Agent/Agent.cpp"
    "Source/Core/GameObject/Controller/Controller.cpp"
    "Source/Core/GameObject/World/World.cpp"
    "Source/Core/GameObject/World/Level/Level.cpp"

    # Type Registry, needed for gameplay actions potentially
    "Source/Core/TypeRegistry/TypeRegistry.cpp"

    # Ticking, more systems to be added later
    "Source/Core/TickManager/TickManager.cpp"

    "Source/Core/Statics/GameStatics.cpp"

    ${REFLECTED_SOURCES}
)

target_compile_definitions(MatrixEngineLib
    PRIVATE   MATRIXENGINE_EXPORTS    # defined when building the dll
    INTERFACE MATRIXENGINE_IMPORTS    # defined when consuming the dll
)

target_link_libraries(MatrixEngineLib
    PUBLIC
        SDL3::SDL3
        SDL3_shadercross::SDL3_shadercross
        SDL3_image::SDL3_image
        glm::glm
        assimp::assimp
        pugixml::pugixml
)

set_target_properties(MatrixEngineLib PROPERTIES
    DEBUG_POSTFIX ""
)

# -----------------------------
# EDITOR
# -----------------------------
add_library(MatrixEditorLib STATIC
    "Source/Engine.cpp"
    "Source/Editor/Editor.cpp"
    "Source/Editor/Viewport/Viewport.cpp"
    "Source/Editor/Viewport/ViewportController/ViewportController.cpp"
    "Source/Editor/Viewport/ViewportCamera/ViewportCamera.cpp"
    "Source/Editor/Viewport/ViewportRenderer/ViewportRenderer.cpp"
    "Source/Editor/EditorRenderer/EditorRenderer.cpp"
    "Source/Editor/ContentBrowser/ContentBrowser.cpp"
    "Source/Editor/ContentBrowser/ContentBrowserController/ContentBrowserController.cpp"
    "Source/Editor/DetailsPanel/DetailsPanel.cpp"
    "Source/Editor/DetailsPanel/DetailsPanelController/DetailsPanelController.cpp"
    "Source/Editor/ProjectSelector/ProjectSelector.cpp"
    "Source/Editor/ProjectLoader/ProjectLoader.cpp"
    "Source/Editor/OutlinerPanel/OutlinerPanel.cpp"
    "Source/Editor/OutlinerPanel/OutlinerPanelController/OutlinerPanelController.cpp"

    # Asset pipeline
    "Source/Core/Assets/AssetLoader/AssetLoader.cpp"
    "Source/Core/Assets/DefaultAssets/DefaultAssets.cpp"

    "Source/Core/Event/EventBUS/EngineEventBUS.cpp"
 )

target_link_libraries(MatrixEditorLib
    PUBLIC
        MatrixEngineLib
        imgui::imgui
#       imguizmo::imguizmo
        nfd::nfd
)

# -----------------------------
# Engine EXECUTABLE
# -----------------------------
add_executable(Matrix
    "Source/main.cpp"
)

target_link_libraries(Matrix
    PRIVATE
        MatrixEngineLib
        MatrixEditorLib
)

# -----------------------------
# C++ standard
# -----------------------------
target_compile_features(Matrix PRIVATE cxx_std_23)

target_compile_definitions(MatrixEditorLib PUBLIC 
    MATRIX_EDITOR
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

# -----------------------------
# Include directories
# -----------------------------
target_include_directories(Matrix
    PRIVATE
        ${MATRIX_ENGINE_ROOT}/Source
)

# -----------------------------
# Dependencies
# -----------------------------

# --- SDL3 ---
add_subdirectory(${MATRIX_ENGINE_ROOT}/vendored/SDL3 EXCLUDE_FROM_ALL)

# --- SDL_image ---
set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_AV1 OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_JXL OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_TIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_HEIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_QOI OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_RENDER ON CACHE BOOL "" FORCE)
set(SDLIMAGE_ZLIB OFF CACHE BOOL "" FORCE)

add_subdirectory(${MATRIX_ENGINE_ROOT}/vendored/SDL_image EXCLUDE_FROM_ALL)

# --- assimp ---
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
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

add_subdirectory(${MATRIX_ENGINE_ROOT}/vendored/assimp EXCLUDE_FROM_ALL)

# target_link_libraries(assimp PRIVATE zlibstatic)

# --- SDL_shadercross ---
# if compilation / hot reloading shaders becomes necessary later(probably) remove if statement and build dxcompiler directly via vendored
if(WIN32)
    set(SDL_SHADERCROSS_HLSL ON CACHE BOOL "" FORCE)
else()
    set(SDL_SHADERCROSS_HLSL OFF CACHE BOOL "" FORCE)
endif()

set(SDLSHADERCROSS_VENDORED ON CACHE BOOL "" FORCE)
set(SDL_SHADERCROSS_SPIRV ON CACHE BOOL "" FORCE)
set(SDL_SHADERCROSS_GLSL ON CACHE BOOL "" FORCE)
set(SDL_SHADERCROSS_MSL ON CACHE BOOL "" FORCE)

add_subdirectory(${MATRIX_ENGINE_ROOT}/vendored/SDL_shadercross EXCLUDE_FROM_ALL)

if(TARGET dxcompiler AND NOT WIN32)
    add_custom_target(dxcompiler_symlink
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            $<TARGET_FILE_NAME:dxcompiler>
            ${CMAKE_BINARY_DIR}/vendored/SDL_shadercross/external/DirectXShaderCompiler/lib/libdxcompiler.so
        COMMENT "Creating unsuffixed symlink for dxcompiler SONAME mismatch"
    )
    add_dependencies(dxcompiler_symlink dxcompiler)

    if(TARGET SDL3_shadercross-shared)
        add_dependencies(SDL3_shadercross-shared dxcompiler_symlink)
    endif()
endif()

add_subdirectory(${MATRIX_ENGINE_ROOT}/vendored/glm EXCLUDE_FROM_ALL)
add_subdirectory(${MATRIX_ENGINE_ROOT}/vendored/imgui EXCLUDE_FROM_ALL)
# add_subdirectory(${MATRIX_ENGINE_ROOT}/vendored/ImGuizmo EXCLUDE_FROM_ALL)
add_subdirectory(${MATRIX_ENGINE_ROOT}/vendored/nfd_extended EXCLUDE_FROM_ALL)
add_subdirectory(${MATRIX_ENGINE_ROOT}/vendored/pugixml EXCLUDE_FROM_ALL)

target_include_directories(MatrixEngineLib
    PUBLIC
        ${MATRIX_ENGINE_ROOT}/Source
        ${CMAKE_BINARY_DIR}/.reflected
)

target_include_directories(MatrixEditorLib
    PUBLIC
        ${MATRIX_ENGINE_ROOT}/Source
)

# -----------------------------
# Install
# -----------------------------

# Engine lib
install(TARGETS MatrixEngineLib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
)

# Vendored static libs
install(FILES
    $<TARGET_FILE:SDL3-shared>
    $<TARGET_LINKER_FILE:SDL3-shared>
    $<TARGET_FILE:SDL3_image::SDL3_image>
    $<TARGET_FILE:zlibstatic>
    $<TARGET_FILE:assimp>
    $<TARGET_FILE:SDL3_shadercross-shared>
    $<TARGET_LINKER_FILE:SDL3_shadercross-shared>
    $<TARGET_FILE:pugixml-static>
    DESTINATION lib
)

# Engine headers
install(DIRECTORY ${MATRIX_ENGINE_ROOT}/Source/
    DESTINATION include/MatrixEngine
    FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY ${CMAKE_BINARY_DIR}/.reflected/
    DESTINATION include/MatrixEngine
    FILES_MATCHING PATTERN "*.reflected.hpp"
)

# Vendored headers
install(DIRECTORY ${MATRIX_ENGINE_ROOT}/vendored/glm/glm/
    DESTINATION include/glm
)

install(DIRECTORY ${MATRIX_ENGINE_ROOT}/vendored/SDL3/include/SDL3/
    DESTINATION include/SDL3
)

install(DIRECTORY ${MATRIX_ENGINE_ROOT}/vendored/SDL_image/include/
    DESTINATION include
)

install(DIRECTORY ${MATRIX_ENGINE_ROOT}/vendored/assimp/include/
    DESTINATION include
)

install(DIRECTORY ${MATRIX_ENGINE_ROOT}/vendored/SDL_shadercross/include/
    DESTINATION include
)

install(FILES ${CMAKE_BINARY_DIR}/vendored/assimp/include/assimp/config.h
    DESTINATION include/assimp
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
        ${MATRIX_ENGINE_ROOT}/shaders
        $<TARGET_FILE_DIR:Matrix>/shaders
    COMMENT "Copying shader source files"
)
# -----------------------------
# Auto-install after build
# -----------------------------

if(CMAKE_CONFIGURATION_TYPES)
    set(INSTALL_CONFIG_ARGS --config $<CONFIG>)
else()
    set(INSTALL_CONFIG_ARGS --config ${CMAKE_BUILD_TYPE})
endif()

add_custom_command(TARGET MatrixEngineLib POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        --install ${CMAKE_BINARY_DIR}
        --prefix "${CMAKE_BINARY_DIR}/$<CONFIG>"
        ${INSTALL_CONFIG_ARGS}
    COMMENT "Installing MatrixEngineLib to build output directory..."
)

# -----------------------------
# Assets copy
# -----------------------------
add_custom_command(TARGET Matrix POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${MATRIX_ENGINE_ROOT}/Content
        $<TARGET_FILE_DIR:Matrix>/Content
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${MATRIX_ENGINE_ROOT}/Source/Engine.png
        $<TARGET_FILE_DIR:Matrix>/Engine.png
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${MATRIX_ENGINE_ROOT}/Matrix.config
        $<TARGET_FILE_DIR:Matrix>/Matrix.config
    COMMENT "Copying assets"
)