#pragma once

#include <vector>
#include <kairo/shader.h>
#include <kairo/selection.h>
#include <kairo/camera.h>
#include <kairo/game_object.h>
#include <kairo/engine_context.h>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// process input
void processInput(GLFWwindow *window, EngineContext& engineContext);

// Set the global context pointer for GLFW callbacks (must be called once before any callbacks fire)
void setInputContext(EngineContext* engineContext);

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

// scroll to change fov
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);