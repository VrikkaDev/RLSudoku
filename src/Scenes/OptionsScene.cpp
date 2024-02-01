//
// Created by VrikkaDev on 31.1.2024.
//

#include "OptionsScene.h"
#include "Event/MouseEvent.h"
#include "Scenes/Drawables/GenericButton.h"
#include "GameData.h"
#include "MainMenuScene.h"
#include "Storage/StorageManager.h"
#include "Scenes/Drawables/ToggleButton.h"

OptionsScene::OptionsScene() : Scene(){

}

void OptionsScene::Setup() {
    // Back button
    float bw = 250, bh = 50, bx = GetScreenWidth()/2 - bw/2, by = GetScreenHeight() - 70;

    auto bb = new GenericButton("Back", Rectangle{bx,by,bw,bh});
    bb->OnClick = [](MouseEvent* event) {
        // Change scene
        GameData::SetScene(std::make_unique<MainMenuScene>());
    };
    drawableStack->AddDrawable(bb);


    float ttw = 250, tth = 50, ttx = GetScreenWidth()/2 - ttw/2, tty = GetScreenHeight() - tth*4;
    auto ttb = new ToggleButton("options_toggle_button", "Toggle", Rectangle{ttx,tty,ttw,tth});
    drawableStack->AddDrawable(ttb);

    float tt2w = 250, tt2h = 50, tt2x = GetScreenWidth()/2 - tt2w/2, tt2y = GetScreenHeight() - tt2h*6;
    auto tt2b = new ToggleButton("options_toggle_button2", "Toggle2", Rectangle{tt2x,tt2y,tt2w,tt2h});
    drawableStack->AddDrawable(tt2b);
}
