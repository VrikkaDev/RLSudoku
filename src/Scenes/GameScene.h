//
// Created by VrikkaDev on 31.1.2024.
//

#ifndef RLSUDOKU_GAMESCENE_H
#define RLSUDOKU_GAMESCENE_H


#include "Scene.h"

class TileGrid;

class GameScene : public Scene{
public:
    GameScene();
    explicit GameScene(int difficulty);

    void Setup() override;

    SudokuBoard board;
    SudokuBoard solvedBoard;

    TileGrid* tileGrid;

    // Difficulty 0-100 not in use currently
    int difficulty = 0;
};


#endif //RLSUDOKU_GAMESCENE_H
