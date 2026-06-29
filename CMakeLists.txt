cmake_minimum_required(VERSION 3.31)
project(Matrix_Engine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

# -----------------------------
# Global output directories
# -----------------------------
set(MATRIX_ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
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
# Engine LIBRARY
# -----------------------------
add_library(MatrixEngineLib SHARED
    # Core engine runtime
    "EngineFiles/Core/Game/Game.cpp"

    # Asset pipeline
    "EngineFiles/Core/Assets/AssetLoader/AssetLoader.cpp"
    "EngineFiles/Core/Assets/DefaultAssets/DefaultAssets.cpp"

    # Rendering
    "EngineFiles/Core/ShaderManager/ShaderManager.cpp"
    "EngineFiles/Core/Editor/Render/WorldRenderer/WorldRenderer.cpp"

    # GameObject system
    "EngineFiles/Core/GameObject/GameObject.cpp"
    "EngineFiles/Core/GameObject/Component/Component.cpp"
    "EngineFiles/Core/GameObject/Component/MeshComponent/MeshComponent.cpp"
    "EngineFiles/Core/GameObject/Component/SpriteComponent/SpriteComponent.cpp"
    "EngineFiles/Core/GameObject/Entity/Entity.cpp"
    "EngineFiles/Core/GameObject/Entity/Agent/Agent.cpp"
    "EngineFiles/Core/GameObject/Controller/Controller.cpp"
    "EngineFiles/Core/GameObject/World/World.cpp"
    "EngineFiles/Core/GameObject/World/Level/Level.cpp"

    # Events, ticking, need to be changed to editor later
    "EngineFiles/Core/Event/EventBUS/EngineEventBUS.cpp"
    "EngineFiles/Core/TickManager/TickManager.cpp"

    # Reflection
    "EngineFiles/Core/Reflection/Read/ReflectFileReader.cpp"
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
    "EngineFiles/Core/Engine.cpp"
    "EngineFiles/Core/Editor/Editor.cpp"
    "EngineFiles/Core/Editor/Viewport/Viewport.cpp"
    "EngineFiles/Core/Editor/Viewport/ViewportController/ViewportController.cpp"
    "EngineFiles/Core/Editor/Viewport/ViewportCamera/ViewportCamera.cpp"
    "EngineFiles/Core/Editor/Render/ViewportRenderer/ViewportRenderer.cpp"
    "EngineFiles/Core/Editor/Render/EditorRenderer/EditorRenderer.cpp"
    "EngineFiles/Core/Editor/ContentBrowser/ContentBrowser.cpp"
    "EngineFiles/Core/Editor/ContentBrowser/ContentBrowserController/ContentBrowserController.cpp"
    "EngineFiles/Core/Editor/ProjectSelector/ProjectSelector.cpp"
 "EngineFiles/Core/Editor/ProjectLoader/ProjectLoader.cpp")

target_link_libraries(MatrixEditorLib
    PUBLIC
        MatrixEngineLib
        imgui::imgui
        nfd::nfd
)

# -----------------------------
# Engine EXECUTABLE
# -----------------------------
add_executable(Matrix
    "EngineFiles/main.cpp"
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
        ${MATRIX_ENGINE_ROOT}/EngineFiles
)

# -----------------------------
# Dependencies
# -----------------------------

# --- SDL3 ---
add_subdirectory(${MATRIX_ENGINE_ROOT}/vendored/SDL3 EXCLUDE_FROM_ALL)

## --- Build zlib once into the EXACT directory SDL_image would use ---
set(ZLIB_SOURCE_DIR "${MATRIX_ENGINE_ROOT}/vendored/SDL_image/external/zlib")
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
set(SDLIMAGE_RENDER ON CACHE BOOL "" FORCE)
set(SDLIMAGE_ZLIB OFF CACHE BOOL "" FORCE)

set(ZLIB_FOUND TRUE CACHE BOOL "" FORCE)
set(ZLIB_LIBRARY zlibstatic CACHE STRING "" FORCE)
set(ZLIB_INCLUDE_DIR "${ZLIB_SOURCE_DIR};${ZLIB_BINARY_DIR}" CACHE PATH "" FORCE)

# Patch SDL_image's CMakeLists to guard its zlib add_subdirectory
set(SDLIMAGE_CMAKE "${MATRIX_ENGINE_ROOT}/vendored/SDL_image/CMakeLists.txt")
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

add_subdirectory(${MATRIX_ENGINE_ROOT}/vendored/SDL_image EXCLUDE_FROM_ALL)

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

add_subdirectory(${MATRIX_ENGINE_ROOT}/vendored/assimp EXCLUDE_FROM_ALL)

target_link_libraries(assimp PRIVATE zlibstatic)

# --- SDL_shadercross ---
set(SDLSHADERCROSS_VENDORED ON CACHE BOOL "" FORCE)
set(SDL_SHADERCROSS_SPIRV ON CACHE BOOL "" FORCE)
set(SDL_SHADERCROSS_GLSL ON CACHE BOOL "" FORCE)
set(SDL_SHADERCROSS_HLSL ON CACHE BOOL "" FORCE)
set(SDL_SHADERCROSS_MSL ON CACHE BOOL "" FORCE)

add_subdirectory(${MATRIX_ENGINE_ROOT}/vendored/SDL_shadercross EXCLUDE_FROM_ALL)
add_subdirectory(${MATRIX_ENGINE_ROOT}/vendored/glm EXCLUDE_FROM_ALL)
add_subdirectory(${MATRIX_ENGINE_ROOT}/vendored/imgui EXCLUDE_FROM_ALL)
add_subdirectory(${MATRIX_ENGINE_ROOT}/vendored/nfd_extended EXCLUDE_FROM_ALL)
add_subdirectory(${MATRIX_ENGINE_ROOT}/vendored/pugixml EXCLUDE_FROM_ALL)

target_include_directories(MatrixEngineLib
    PUBLIC
        ${MATRIX_ENGINE_ROOT}/EngineFiles
)

target_include_directories(MatrixEditorLib
    PUBLIC
        ${MATRIX_ENGINE_ROOT}/EngineFiles
)

# -----------------------------
# Install
# -----------------------------

# Engine lib
install(TARGETS MatrixEngineLib
    ARCHIVE DESTINATION lib    # the .lib import file
    RUNTIME DESTINATION lib    # the .dll
)

# Vendored static libs — use ARCHIVE for static, check exact target names
install(FILES
    $<TARGET_FILE:SDL3-shared>
    $<TARGET_LINKER_FILE:SDL3-shared>
    $<TARGET_FILE:SDL3_image-static>
    $<TARGET_FILE:zlibstatic>
    $<TARGET_FILE:assimp>
    $<TARGET_FILE:SDL3_shadercross-shared>
    $<TARGET_LINKER_FILE:SDL3_shadercross-shared>
    $<TARGET_FILE:pugixml-static>
    DESTINATION lib
)

# Engine headers
install(DIRECTORY ${MATRIX_ENGINE_ROOT}/EngineFiles/
    DESTINATION include/MatrixEngine
    FILES_MATCHING PATTERN "*.hpp"
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
install(FILES ${CMAKE_BINARY_DIR}/vendored/assimp/include/assimp/config.h
    DESTINATION include/assimp
)
install(DIRECTORY ${MATRIX_ENGINE_ROOT}/vendored/SDL_shadercross/include/
    DESTINATION include
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
add_custom_command(TARGET MatrixEngineLib POST_BUILD
    COMMAND ${CMAKE_COMMAND} 
        --install ${CMAKE_BINARY_DIR}
        --config $<CONFIG>
        --prefix ${MATRIX_ENGINE_ROOT}/install
    COMMENT "Installing MatrixEngineLib to install directory..."
)

# -----------------------------
# Assets copy
# -----------------------------
add_custom_command(TARGET Matrix POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${MATRIX_ENGINE_ROOT}/Content
        $<TARGET_FILE_DIR:Matrix>/Content
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${MATRIX_ENGINE_ROOT}/EngineFiles/Engine.png
        $<TARGET_FILE_DIR:Matrix>/Engine.png
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${MATRIX_ENGINE_ROOT}/Matrix.config
        $<TARGET_FILE_DIR:Matrix>/Matrix.config
    COMMENT "Copying assets"
)