//
// Created by VrikkaDev on 31.1.2024.
//

#include "GameScene.h"
#include "Graphics/Scenes/Drawables/GenericButton.h"
#include "GameData.h"
#include "MainMenuScene.h"
#include "Graphics/Scenes/Drawables/TileButton.h"
#include "Graphics/Scenes/Drawables/TileGrid.h"

GameScene::GameScene() : Scene(){

}

GameScene::GameScene(int difficulty) : Scene() {
    this->difficulty = difficulty;
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

    auto tb = new TileGrid(Rectangle{tx,ty,tw,th});
    drawableStack->AddDrawable(tb);
}
