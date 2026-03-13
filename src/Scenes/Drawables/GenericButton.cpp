//
// Created by VrikkaDev on 30.1.2024.
//

#include "GenericButton.h"
#include "GameData.h"
#include "Storage/StorageManager.h"

namespace {
bool IsDarkModeEnabled() {
    if (!GameData::storageManager) {
        return false;
    }
    nlohmann::json mode = GameData::storageManager->GetData("options_toggle_darkmode");
    return mode.contains("value") && mode["value"].is_boolean() && mode["value"];
}
}

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

    if (IsDarkModeEnabled()) {
        if (color.r == GRAY.r && color.g == GRAY.g && color.b == GRAY.b && color.a == GRAY.a) {
            color = CLITERAL(Color){55, 55, 62, 255};
        }
        if (hoverColor.r == DARKGRAY.r && hoverColor.g == DARKGRAY.g && hoverColor.b == DARKGRAY.b && hoverColor.a == DARKGRAY.a) {
            hoverColor = CLITERAL(Color){75, 75, 84, 255};
        }
        if (pressColor.r == LIGHTGRAY.r && pressColor.g == LIGHTGRAY.g && pressColor.b == LIGHTGRAY.b && pressColor.a == LIGHTGRAY.a) {
            pressColor = CLITERAL(Color){95, 95, 106, 255};
        }
        if (textColor.r == WHITE.r && textColor.g == WHITE.g && textColor.b == WHITE.b && textColor.a == WHITE.a) {
            textColor = CLITERAL(Color){230, 230, 236, 255};
        }
    } else {
        if (color.r == 55 && color.g == 55 && color.b == 62 && color.a == 255) {
            color = GRAY;
        }
        if (hoverColor.r == 75 && hoverColor.g == 75 && hoverColor.b == 84 && hoverColor.a == 255) {
            hoverColor = DARKGRAY;
        }
        if (pressColor.r == 95 && pressColor.g == 95 && pressColor.b == 106 && pressColor.a == 255) {
            pressColor = LIGHTGRAY;
        }
        if (textColor.r == 230 && textColor.g == 230 && textColor.b == 236 && textColor.a == 255) {
            textColor = WHITE;
        }
    }

    bool isHovering = CheckCollisionPointRec(GetMousePosition(), GetRectangle());
    bool isPressed = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

    DrawRectangle(x, y, width, height, isHovering ? isPressed ? pressColor : hoverColor : color);
    DrawTextBC(text.c_str(), x, y, fontSize, width, height, textColor);
}

