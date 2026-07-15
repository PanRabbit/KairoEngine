cmake_minimum_required(VERSION 3.20)
project(KairoEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force ImGui to use your GLAD loader explicitly
add_compile_definitions(IMGUI_IMPL_OPENGL_LOADER_GLAD)

# ==============================================================================
# 1. COMPILE IMGUI AS AN ISOLATED STATIC LIBRARY
# ==============================================================================
# This creates a separate target called "imgui_lib". 
# The compiler builds this target once, creates a binary archive, and caches it.
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
# 2. YOUR ACTIVE ENGINE SOURCE FILES (The only things you edit)
# ==============================================================================
set(PROJECT_SOURCES
    main.cpp
    include/stb/stb_implementation.cpp
    include/kairo/UI.cpp
    include/kairo/texture.cpp
    include/kairo/material.cpp
)

# ==============================================================================
# 3. PLATFORM DEPENDENCIES & LINKING
# ==============================================================================
if(WIN32)
    # Windows/Vcpkg setup
    find_package(glad CONFIG REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)
    find_package(glm CONFIG REQUIRED)

    add_executable(KairoEngine ${PROJECT_SOURCES})
    target_include_directories(KairoEngine PRIVATE imgui imgui/backend)
    
    # Link your engine to your custom cached library alongside system dependencies
    target_link_libraries(KairoEngine PRIVATE imgui_lib glad::glad glfw glm::glm)
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

    # Link everything together, attaching your compiled imgui_lib
    target_link_libraries(KairoEngine PRIVATE imgui_lib glfw glm::glm OpenGL::GL ${CMAKE_DL_LIBS})
endif()