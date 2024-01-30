//
// Created by VrikkaDev on 30.1.2024.
//

#ifndef RLSUDOKU_TEXTHELPER_H
#define RLSUDOKU_TEXTHELPER_H

#include "raylib.h"

struct Fonts{
    static Font default_font;
};

// Draw text using font
void DrawTextB(const char *text, int posX, int posY, int fontSize, Color color);
// Draw text using font and centered
void DrawTextBC(const char *text, int posX, int posY, int fontSize, int width, int height, Color color);

#endif //RLSUDOKU_TEXTHELPER_H
