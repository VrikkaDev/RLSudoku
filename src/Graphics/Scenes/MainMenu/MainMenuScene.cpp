//
// Created by VrikkaDev on 29.1.2024.
//

#include "MainMenuScene.h"
#include "Graphics/Scenes/MainMenu/Drawables/ExitButton.h"

MainMenuScene::MainMenuScene() : Scene() {
}

void MainMenuScene::Setup() {

    drawableStack->AddDrawable(new ExitButton());
}

MainMenuScene::~MainMenuScene() = default;
