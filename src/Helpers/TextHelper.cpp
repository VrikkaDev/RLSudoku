//
// Created by VrikkaDev on 30.1.2024.
//

#include "TextHelper.h"

Font Fonts::roboto_black = {};

void DrawTextB(const char *text, int posX, int posY, int fontSize, Color color){
    DrawTextEx(Fonts::roboto_black, text, (Vector2){ static_cast<float>(posX), static_cast<float>(posY) }, fontSize, 1, color);
}

