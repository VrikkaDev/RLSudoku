//
// Created by VrikkaDev on 29.1.2024.
//

#include "GameData.h"
#include "Scenes/Scene.h"

std::unique_ptr<Scene> GameData::currentScene = std::make_unique<Scene>();
bool GameData::isRunning = true;