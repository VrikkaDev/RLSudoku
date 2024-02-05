//
// Created by VrikkaDev on 1.2.2024.
//

#include "Saveable.h"
#include "GameData.h"
#include "StorageManager.h"
#include "Scenes/Scene.h"

Saveable::Saveable(const char* save_tkn) {
    save_token = save_tkn;

    GameData::currentScene->saveableStack.AddSaveable(std::unique_ptr<Saveable>(this));
}
