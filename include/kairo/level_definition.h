#pragma once

#include <kairo/engine_context.h>
#include <string>

void LoadLevelFromJson(EngineContext& engineContext, const std::string& path);
void SaveLevelToJson(EngineContext& engineContext, const std::string& path);
