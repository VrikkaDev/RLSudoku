//
// Created by VrikkaDev on 29.1.2024.
//

#include "Sudoku.h"
#include "Graphics/Scenes/Scene.h"
#include "Event/MouseEvent.h"
#include "Graphics/Scenes/MainMenu/MainMenuScene.h"

void Sudoku::Run() {

    GameData::currentScene.reset();
    GameData::currentScene = std::make_unique<MainMenuScene>();
    GameData::currentScene->Setup();

    int btnState = 0; // Button state: 0-NORMAL, 1-LEFT_CLICK, 2-RIGHT_CLICK
    Vector2 mousePoint {0,0};

    // Main game loop
    while (!WindowShouldClose())// Detect window close button or ESC key
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        EndDrawing();

        mousePoint = GetMousePosition();

        // Check button state
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) btnState = 1;
        else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) btnState = 2;
        else btnState = 0;

        if (btnState != 0) {
            auto* event = new MouseEvent(btnState, mousePoint);
            GameData::currentScene->drawableStack->OnEvent(event);
        }

        GameData::currentScene->drawableStack->Draw();
    }
}
