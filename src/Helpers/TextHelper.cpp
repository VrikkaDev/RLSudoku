//
// Created by VrikkaDev on 30.1.2024.
//

#include "TextHelper.h"

Font Fonts::default_font = {};

void DrawTextB(const char *text, int posX, int posY, int fontSize, Color color){
    DrawTextEx(Fonts::default_font, text, (Vector2){static_cast<float>(posX), static_cast<float>(posY) }, fontSize, 1, color);
}

void DrawTextBC(const char *text, int posX, int posY, int fontSize, int width, int height, Color color){

    int textWidth = MeasureText(text, height);  // Measure the width based on the text height
    int textHeight = height;  // Use the same height as the rectangle

    // Calculate the position to center the text in the rectangle
    int textX = posX + (width - textWidth) / 2;
    int textY = posY + (height - textHeight) / 2;

    DrawTextEx(Fonts::default_font, text, (Vector2){static_cast<float>(textX), static_cast<float>(textY) }, fontSize, 1, color);
}

