//
// Created by VrikkaDev on 31.1.2024.
//

#ifndef RLSUDOKU_GAMESCENE_H
#define RLSUDOKU_GAMESCENE_H


#include "Scene.h"

class GameScene : public Scene{
public:
    GameScene();
    explicit GameScene(int difficulty);

    void Setup() override;

    // Difficulty 0-100
    int difficulty = 0;
};


#endif //RLSUDOKU_GAMESCENE_H
