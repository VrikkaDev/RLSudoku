//
// Created by VrikkaDev on 29.1.2024.
//

#ifndef RLSUDOKU_GAMEDATA_H
#define RLSUDOKU_GAMEDATA_H

#include "pch.hxx"

class Scene;
class StorageManager;
class LeaderboardManager;
class StatisticsManager;
class RemoteSyncManager;

class GameData {
public:
    static bool isRunning;
    static std::unique_ptr<Scene> currentScene;
    static std::unique_ptr<StorageManager> storageManager;
    static std::unique_ptr<LeaderboardManager> leaderboardManager;
    static std::unique_ptr<StatisticsManager> statisticsManager;
    static std::unique_ptr<RemoteSyncManager> remoteSyncManager;

    // prob should use some sort of SceneManager instead of having these here
    static void SetScene(std::unique_ptr<Scene> scene);
    static void HandleSceneChange();

private:
    static bool changeScene;
    static std::unique_ptr<Scene> nextScene;
};

#endif //RLSUDOKU_GAMEDATA_H
