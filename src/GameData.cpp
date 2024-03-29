//
// Created by VrikkaDev on 29.1.2024.
//

#include "GameData.h"
#include "Scenes/Scene.h"
#include "Storage/StorageManager.h"

std::unique_ptr<Scene> GameData::currentScene = std::make_unique<Scene>();
std::unique_ptr<Scene> GameData::nextScene = nullptr;
std::unique_ptr<StorageManager> GameData::storageManager = std::make_unique<StorageManager>();
bool GameData::isRunning = true;
bool GameData::changeScene = false;

void GameData::SetScene(std::unique_ptr<Scene> scene) {
    nextScene = std::move(scene);
    changeScene = true;
}

void GameData::HandleSceneChange() {
    if(changeScene){

        // Save and clear saveables
        storageManager->Save();
        currentScene->saveableStack.Flush();

        currentScene.reset();
        currentScene = std::move(nextScene);
        if(!currentScene->DidSetup){
            currentScene->Setup();
        }
        currentScene->DidSetup = true;

        // Load saveables
        storageManager->Load();

        nextScene = nullptr;
        changeScene = false;
    }
}
