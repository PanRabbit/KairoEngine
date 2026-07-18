cmake_minimum_required(VERSION 3.20)
project(KairoEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force ImGui to use your GLAD loader explicitly
add_compile_definitions(IMGUI_IMPL_OPENGL_LOADER_GLAD)

# ==============================================================================
# 1. FETCH ASSIMP (CROSS-PLATFORM)
# ==============================================================================
include(FetchContent)

# Turn off Assimp bloat so it compiles faster and doesn't cause OS conflicts
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Disable tests" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "Disable tools" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "Build zlib internally" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "Don't fail on warnings" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "Disable installation rules" FORCE)

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v5.3.1
)
FetchContent_MakeAvailable(assimp)

# ==============================================================================
# 2. COMPILE IMGUI AS AN ISOLATED STATIC LIBRARY
# ==============================================================================
add_library(imgui_lib STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_demo.cpp
    imgui/backend/imgui_impl_glfw.cpp
    imgui/backend/imgui_impl_opengl3.cpp
)

# ==============================================================================
# 3. YOUR ACTIVE ENGINE SOURCE FILES
# ==============================================================================
set(PROJECT_SOURCES
    main.cpp
    include/stb/stb_implementation.cpp
    include/kairo/UI.cpp
    include/kairo/texture.cpp
    include/kairo/material.cpp
)

# ==============================================================================
# 4. PLATFORM DEPENDENCIES & LINKING
# ==============================================================================
if(WIN32)
    # Windows/Vcpkg setup
    find_package(glad CONFIG REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)
    find_package(glm CONFIG REQUIRED)

    add_executable(KairoEngine ${PROJECT_SOURCES})
    target_include_directories(KairoEngine PRIVATE imgui imgui/backend)
    
    # Linked assimp here at the end
    target_link_libraries(KairoEngine PRIVATE imgui_lib glad::glad glfw glm::glm assimp)
else()
    # Linux setup
    find_package(PkgConfig REQUIRED)
    find_package(glfw3 REQUIRED)
    find_package(glm REQUIRED)
    find_package(OpenGL REQUIRED)

    # Compile KairoEngine with your local glad file
    add_executable(KairoEngine ${PROJECT_SOURCES} src/glad.c)

    # Let BOTH your main executable and the static library see the headers
    target_include_directories(KairoEngine PRIVATE include . imgui imgui/backend)
    target_include_directories(imgui_lib PRIVATE include . imgui imgui/backend)

    # Linked assimp here at the end
    target_link_libraries(KairoEngine PRIVATE imgui_lib glfw glm::glm OpenGL::GL ${CMAKE_DL_LIBS} assimp)
endif()