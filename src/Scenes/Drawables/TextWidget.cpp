//
// Created for RLSudoku Static Text Display
//

#include "TextWidget.h"
#include "Helpers/TextHelper.h"
#include "Helpers/UIHelper.h"

namespace {

Color ResolveTextColor(Color baseColor, bool darkMode) {
    if (!darkMode) {
        return baseColor;
    }

    if (baseColor.r == BLACK.r && baseColor.g == BLACK.g && baseColor.b == BLACK.b && baseColor.a == BLACK.a) {
        return CLITERAL(Color){235, 235, 242, 255};
    }
    if (baseColor.r == DARKGRAY.r && baseColor.g == DARKGRAY.g && baseColor.b == DARKGRAY.b && baseColor.a == DARKGRAY.a) {
        return CLITERAL(Color){208, 208, 218, 255};
    }
    if (baseColor.r == GRAY.r && baseColor.g == GRAY.g && baseColor.b == GRAY.b && baseColor.a == GRAY.a) {
        return CLITERAL(Color){190, 190, 202, 255};
    }

    return baseColor;
}
}

TextWidget::TextWidget(const std::string& text, int x, int y, int fontSize, Color color, bool centered)
    : Drawable() {
    this->text = text;
    this->x = x;
    this->y = y;
    this->fontSize = fontSize;
    this->color = color;
    this->centered = centered;
}

void TextWidget::Draw() {
    Color drawColor = ResolveTextColor(color, UIHelper::IsDarkModeEnabled());
    if (centered) {
        int textWidth = MeasureText(text.c_str(), fontSize);
        DrawTextBC(text.c_str(), x - textWidth / 2, y, fontSize, textWidth, fontSize, drawColor);
    } else {
        DrawTextB(text.c_str(), x, y, fontSize, drawColor);
    }
}
