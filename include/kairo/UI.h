#pragma once

// Tell GLFW not to include the legacy OpenGL headers itself
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "kairo/engine_context.h"

// Function declarations
void InitUI(GLFWwindow* window);
void RenderUI(EngineContext& engineContext);
void ShutdownUI();