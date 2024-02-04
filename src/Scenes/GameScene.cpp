//
// Created by VrikkaDev on 31.1.2024.
//

#include "GameScene.h"
#include "Scenes/Drawables/GenericButton.h"
#include "GameData.h"
#include "MainMenuScene.h"
#include "Scenes/Drawables/TileButton.h"
#include "Scenes/Drawables/TileGrid.h"
#include "Scenes/Drawables/ClockWidget.h"
#include "Scenes/Drawables/NumberButtons.h"

#include "JCZSolve.h"
#include "Scenes/Drawables/WinScreen.h"

GameScene::GameScene() : Scene(){
}

GameScene::GameScene(int difficulty) : Scene() {
    this->difficulty = difficulty;

    board = genBoard(difficulty);

    char solvstr[81];
    std::string st = board.parser();
    JCZSolver(st.c_str(), solvstr, 5);

    solvedBoard = SudokuBoard(solvstr);
}

void GameScene::Setup() {

    // Sudoku Tilegrid
    float tw = 400, th = 400, tx = 20, ty = GetScreenHeight()/2 - th/2;
    auto tb = new TileGrid(&board, &solvedBoard, Rectangle{tx,ty,tw,th});
    tileGrid = tb;
    drawableStack->AddDrawable(tb);

    // Game Clock
    float cw = 200, ch = 50, cx = GetScreenWidth() - 300 - 70, cy = GetScreenHeight() - ch - 20;
    auto cb = new ClockWidget(Rectangle{cx,cy,cw,ch});
    drawableStack->AddDrawable(cb);

    // Number Buttons
    float nw = 320, nh = 320, nx = GetScreenWidth() - nw - 20, ny = 20;
    auto nb = new NumberButtons(Rectangle{nx,ny,nw,nh});
    drawableStack->AddDrawable(nb);

    // Win Screen
    float ww = GetScreenWidth(), wh = GetScreenHeight(), wx = 0, wy = 0;
    auto wb = new WinScreen(Rectangle{wx,wy,ww,wh});
    drawableStack->AddDrawable(wb);

    // Back button
    float bw = 130, bh = 50, bx = GetScreenWidth() - bw - 20, by = GetScreenHeight() - bh - 20;
    auto bb = new GenericButton("Back", Rectangle{bx,by,bw,bh});
    bb->OnClick = [](MouseEvent* event) {
        GameData::SetScene(std::make_unique<MainMenuScene>());
    };
    drawableStack->AddDrawable(bb);
}
