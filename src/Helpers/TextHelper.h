//
// Created by VrikkaDev on 30.1.2024.
//

#ifndef RLSUDOKU_TEXTHELPER_H
#define RLSUDOKU_TEXTHELPER_H

#include "raylib.h"
#include "StringHelper.h"

struct Fonts{
    static Font default_font;
};

// Draw text using font
void DrawTextB(const char* text, int posX, int posY, int fontSize, Color color);
// Draw text using font and centered
void DrawTextBC(const char* text, int posX, int posY, int fontSize, int width, int height, Color color);
// Draw text using font, centered left
void DrawTextBCL(const char* text, int posX, int posY, int fontSize, int height, Color color);
// Draws tooltip at mouse position using font
void DrawTooltipB(const char* text, int fontSize, Color bgColor, Color color);

#endif //RLSUDOKU_TEXTHELPER_H
