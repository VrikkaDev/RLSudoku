//
// Created by VrikkaDev on 30.1.2024.
//

#ifndef RLSUDOKU_TEXTHELPER_H
#define RLSUDOKU_TEXTHELPER_H

#include "raylib.h"

struct Fonts{
    static Font roboto_black;
};

void DrawTextB(const char *text, int posX, int posY, int fontSize, Color color);

#endif //RLSUDOKU_TEXTHELPER_H
