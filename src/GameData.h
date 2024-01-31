//
// Created by VrikkaDev on 29.1.2024.
//

#ifndef RLSUDOKU_GAMEDATA_H
#define RLSUDOKU_GAMEDATA_H

#include "pch.hxx"

class Scene;

class GameData {
public:
    static bool isRunning;
    static std::unique_ptr<Scene> currentScene;
};

#endif //RLSUDOKU_GAMEDATA_H
