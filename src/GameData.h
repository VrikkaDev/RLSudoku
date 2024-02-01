//
// Created by VrikkaDev on 29.1.2024.
//

#ifndef RLSUDOKU_GAMEDATA_H
#define RLSUDOKU_GAMEDATA_H

#include "pch.hxx"

class Scene;
class StorageManager;

class GameData {
public:
    static bool isRunning;
    static std::unique_ptr<Scene> currentScene;
    static std::unique_ptr<StorageManager> storageManager;

    static void SetScene(std::unique_ptr<Scene> scene);

    static void HandleSceneChange();

private:
    static bool changeScene;
    static std::unique_ptr<Scene> nextScene;
};

#endif //RLSUDOKU_GAMEDATA_H
