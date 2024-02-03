//
// Created by VrikkaDev on 29.1.2024.
//

#include "MainMenuScene.h"
#include "Scenes/Drawables/GenericButton.h"
#include "GameData.h"
#include "OptionsScene.h"
#include "Scenes/Drawables/GenericDropdown.h"
#include "GameScene.h"

MainMenuScene::MainMenuScene() : Scene() {
}

void MainMenuScene::Setup() {

    // Options button
    float ow = 220, oh = 50, ox = GetScreenWidth()/2 - ow/2, oy = GetScreenHeight()/2 - oh/2;
    auto ob = new GenericButton("Options", Rectangle{ox,oy,ow,oh});
    ob->OnClick = [](MouseEvent* event) {
        GameData::SetScene(std::make_unique<OptionsScene>());
    };
    drawableStack->AddDrawable(ob);

    // Quit button
    float qw = 190, qh = 50, qx = GetScreenWidth()/2 - qw/2, qy = GetScreenHeight()/2 - qh/2 + 70;
    auto qb = new GenericButton("Quit", Rectangle{qx,qy,qw,qh});
    qb->OnClick = [](MouseEvent* event) {
        GameData::isRunning = false;
    };
    drawableStack->AddDrawable(qb);


    // Difficulty dropdown
    float dw = 200, dh = 50, dx = GetScreenWidth()/2 - dw/2 + 260, dy = GetScreenHeight()/2 - dh/2 - 70;
    auto db = new GenericDropdown({{"Easy", 20}, {"Medium", 40}, {"Hard", 50}, {"Very Hard", 60}}, Rectangle{dx,dy,dw,dh});
    drawableStack->AddDrawable(db);

    // Generate game button
    float gw = 250, gh = 50, gx = GetScreenWidth()/2 - gw/2, gy = GetScreenHeight()/2 - gh/2 - 70;
    auto gb = new GenericButton("Generate", Rectangle{gx,gy,gw,gh});
    gb->OnClick = [db](MouseEvent* event) {
        GameData::SetScene(std::make_unique<GameScene>(db->GetSelectedValue()));
    };
    drawableStack->AddDrawable(gb);
}

MainMenuScene::~MainMenuScene() = default;
