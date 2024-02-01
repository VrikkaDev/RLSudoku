//
// Created by VrikkaDev on 30.1.2024.
//

#include "GenericButton.h"

GenericButton::GenericButton() : Drawable() {
    width = 300;
    height = 50;

    x = GetScreenWidth()/2 - width/2;
    y = GetScreenHeight()/2 - height/2;
}

GenericButton::GenericButton(const char* txt, Rectangle rec){
    text = txt;
    x = rec.x;
    y = rec.y;
    width = rec.width;
    height = rec.height;
}

void GenericButton::Draw() {

    bool isHovering = CheckCollisionPointRec(GetMousePosition(), GetRectangle());
    bool isPressed = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

    DrawRectangle(x, y, width, height, isHovering ? isPressed ? pressColor : hoverColor : color);
    DrawTextBC(text.c_str(), x, y, fontSize, width, height, textColor);
}

