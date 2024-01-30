//
// Created by VrikkaDev on 29.1.2024.
//

#include "MainMenuScene.h"
#include "Graphics/Scenes/MainMenu/Drawables/GenericButton.h"

MainMenuScene::MainMenuScene() : Scene() {
}

void MainMenuScene::Setup() {

    float gw = 500;
    float gh = 50;
    float gx = GetScreenWidth()/2 - gw/2;
    float gy = GetScreenHeight()/2 - gh/2 + 70;

    auto gb = new GenericButton("Generate", Rectangle{gx,gy,gw,gh});
    gb->OnClick = [](MouseEvent* event) {
    };
    drawableStack->AddDrawable(gb);

    float ow = 300;
    float oh = 50;
    float ox = GetScreenWidth()/2 - ow/2;
    float oy = GetScreenHeight()/2 - oh/2;

    auto ob = new GenericButton("Options", Rectangle{ox,oy,ow,oh});
    ob->OnClick = [](MouseEvent* event) {
    };
    drawableStack->AddDrawable(ob);
}

MainMenuScene::~MainMenuScene() = default;
