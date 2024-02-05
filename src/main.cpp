#include <iostream>
#include "pch.hxx"
#include "Game.h"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 480;

    // Setup raylib stuff
    InitWindow(screenWidth, screenHeight, "Simple raylib sudoku");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    // Load default font
    Fonts::default_font = LoadFontEx("./assets/fonts/Roboto-Black.ttf", 128-32, NULL, 0);
    SetTextureFilter(Fonts::default_font.texture, TEXTURE_FILTER_BILINEAR);

    // Run the game
    Game::Run();

    // Close window and gl context
    CloseWindow();

    return 0;
}
