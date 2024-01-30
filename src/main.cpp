#include <iostream>
#include "pch.hxx"
#include "Sudoku.h"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 440;

    // Setup raylib stuff
    InitWindow(screenWidth, screenHeight, "Simple raylib sudoku");
    SetTargetFPS(60);

    Fonts::default_font = LoadFontEx("../assets/fonts/Roboto-Black.ttf", 96, 0, 0);

    // Run the game
    Sudoku::Run();

    // Close window and gl context
    CloseWindow();

    return 0;
}
