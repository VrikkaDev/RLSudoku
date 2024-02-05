//
// Created by VrikkaDev on 31.1.2024.
//

#ifndef RLSUDOKU_GAMESCENE_H
#define RLSUDOKU_GAMESCENE_H


#include "Scene.h"
#include "Storage/Saveable.h"

class TileGrid;
class GameSaveable;

class GameScene : public Scene{
public:
    GameScene();
    explicit GameScene(int difficulty);
    explicit GameScene(bool load);

    void Setup() override;
    void OnResize() override;

    std::unique_ptr<SudokuBoard> board;
    std::unique_ptr<SudokuBoard> orgBoard;
    std::unique_ptr<SudokuBoard> solvedBoard;

    // :(
    TileGrid* tileGrid;
    GameSaveable* gs;
    bool newGame = true; // if false then load game from json.

    // Difficulty 0-100
    int difficulty = 0;
};

class GameSaveable : public Saveable{
public:
    explicit GameSaveable(GameScene* scene);
    GameScene* scene;

    nlohmann::json GetJson() override;
    void Load(const nlohmann::json& data) override;
};


#endif //RLSUDOKU_GAMESCENE_H
