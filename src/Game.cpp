//
// Created by VrikkaDev on 29.1.2024.
//

#include "Game.h"
#include "Scenes/Scene.h"
#include "Event/MouseEvent.h"
#include "Scenes/MainMenuScene.h"
#include "Storage/StorageManager.h"
#include "Event/KeyboardEvent.h"
#include "Scenes/OptionsScene.h"

void Game::Run() {

    // Load configs
    GameData::SetScene(std::make_unique<OptionsScene>());
    GameData::HandleSceneChange();

    GameData::SetScene(std::make_unique<MainMenuScene>());
    GameData::HandleSceneChange();

    int btnState = 0; // Button state: 0-NORMAL, 1-LEFT_CLICK, 2-RIGHT_CLICK
    int prevBtnState = 0; // Previous mouse button state
    Vector2 mousePoint {0,0};

    int keyboardKey = 0;

    // Main game loop
    while (!WindowShouldClose() && GameData::isRunning)// Detect window close button or ESC key
    {
        BeginDrawing();
        ClearBackground(CLITERAL(Color){ 220, 220, 220, 255 } );
        EndDrawing();

        mousePoint = GetMousePosition();

        prevBtnState = btnState;

        keyboardKey = GetKeyPressed();

        if(keyboardKey != 0){
            auto* keybEvent = new KeyboardEvent(1, keyboardKey);
            GameData::currentScene->eventDispatcher->AddEvent(keybEvent);
        }

        // Check button state
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) btnState = 1;
        else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) btnState = 2;
        else btnState = 0;

        if (btnState != 0 || prevBtnState != 0) {

            // 0-NONE, 1-CLICKED, 2-RELEASED
            int eventType = 0;
            // Calculate eventType using previous btn state

            if (prevBtnState == 0){
                eventType = 1;
            }else if(btnState == 0){
                eventType = 2;
            }

            auto* event = new MouseEvent(eventType, btnState, mousePoint);
            GameData::currentScene->eventDispatcher->AddEvent(event);
        }

        // Handle update
        GameData::currentScene->OnUpdate();

        bool ishovering = GameData::currentScene->drawableStack->IsHovering(mousePoint);

        if(ishovering){
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        } else{
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }

        // Handle draw
        GameData::currentScene->drawableStack->Draw();

        GameData::HandleSceneChange();
    }

    // Save data
    GameData::storageManager->Save();

}
