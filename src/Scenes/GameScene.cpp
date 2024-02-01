//
// Created by VrikkaDev on 31.1.2024.
//

#include "GameScene.h"
#include "Scenes/Drawables/GenericButton.h"
#include "GameData.h"
#include "MainMenuScene.h"
#include "Scenes/Drawables/TileButton.h"
#include "Scenes/Drawables/TileGrid.h"
#include "Helpers/SudokuHelper.h"

GameScene::GameScene() : Scene(){

}

GameScene::GameScene(int difficulty) : Scene() {
    this->difficulty = difficulty;

    // TODO Create own sudoku generator/solver for better difficulties

    auto diff = sudoku::PuzzleDifficulty::Easy;
    if(difficulty > 30) diff = sudoku::PuzzleDifficulty::Medium;
    if(difficulty > 60) diff = sudoku::PuzzleDifficulty::Hard;

    auto sdg = SudokuHelper::GenerateSudoku(diff, board);
    if(sdg == sudoku::GeneratorResult::AsyncGenCancelled){
        std::cout<<"AsyncGenCancelled"<<std::endl;
    }

}

void GameScene::Setup() {

    float bw = 150, bh = 50, bx = GetScreenWidth() - bw - 20, by = GetScreenHeight() - bh - 20;

    auto bb = new GenericButton("Back", Rectangle{bx,by,bw,bh});
    bb->OnClick = [](MouseEvent* event) {
        GameData::currentScene = std::make_unique<MainMenuScene>();
        GameData::currentScene->Setup();
    };
    drawableStack->AddDrawable(bb);


    float tw = 400, th = 400, tx = 20, ty = GetScreenHeight()/2 - th/2;

    auto tb = new TileGrid(&board, &solver, Rectangle{tx,ty,tw,th});
    drawableStack->AddDrawable(tb);
}
