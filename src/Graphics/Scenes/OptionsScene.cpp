//
// Created by VrikkaDev on 31.1.2024.
//

#include "OptionsScene.h"
#include "Event/MouseEvent.h"
#include "Graphics/Scenes/Drawables/GenericButton.h"
#include "GameData.h"
#include "MainMenuScene.h"

OptionsScene::OptionsScene() : Scene(){

}

void OptionsScene::Setup() {
    float bw = 250, bh = 50, bx = GetScreenWidth()/2 - bw/2, by = GetScreenHeight()/2 - bh/2 + 70;

    auto bb = new GenericButton("Back", Rectangle{bx,by,bw,bh});
    bb->OnClick = [](MouseEvent* event) {
        GameData::currentScene = std::make_unique<MainMenuScene>();
        GameData::currentScene->Setup();
    };
    drawableStack->AddDrawable(bb);
}
