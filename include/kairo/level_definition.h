#pragma once

#include <kairo/engine_context.h>
#include <string>
#include <vector>
#include <map>

void LoadLevelFromJson(EngineContext& engineContext, const std::string& path);
void SaveLevelToJson(EngineContext& engineContext, const std::string& path);
bool SaveLevelAs(EngineContext& engineContext, const std::string& rawName);
std::string MakeLevelPath(const std::string& rawName);

std::vector<std::string> ListLevelFiles();
std::vector<std::string> ListSkyboxNames();
std::vector<std::string> ListModelNames(const EngineContext& engineContext);
std::map<std::string, std::vector<std::string>> ListModelsByFolder(const EngineContext& engineContext);
std::vector<std::string> ListMaterialNames(const EngineContext& engineContext);

void AddObjectToLevel(EngineContext& engineContext, const std::string& modelName);
void AddPointLightToLevel(EngineContext& engineContext);
void AddSpotLightToLevel(EngineContext& engineContext);
void DuplicateSelectedObject(EngineContext& engineContext);
void DeleteSelectedFromLevel(EngineContext& engineContext);
