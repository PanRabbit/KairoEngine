#pragma once

#include <kairo/shader.h>
#include <kairo/engine_context.h>

void CreateSunDepthMapFBO(unsigned int& depthMapFBO, unsigned int& depthMapTexture);
void RenderSceneToDepthMap(EngineContext& engineContext);
void InitPointLightCubemaps(EngineContext& engineContext);
void RenderSceneToDepthCubemap(EngineContext& engineContext, unsigned int lightIndex);