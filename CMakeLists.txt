cmake_minimum_required(VERSION 3.20)
project(KairoEngine)

set(CMAKE_CXX_STANDARD 17)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(KairoEngine main.cpp)

target_link_libraries(KairoEngine PRIVATE glad::glad glfw glm::glm)