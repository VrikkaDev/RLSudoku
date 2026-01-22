//
// Created for RLSudoku Static Text Display
//

#include "TextWidget.h"
#include "Helpers/TextHelper.h"

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
    if (centered) {
        int textWidth = MeasureText(text.c_str(), fontSize);
        DrawTextBC(text.c_str(), x - textWidth / 2, y, fontSize, textWidth, fontSize, color);
    } else {
        DrawTextBC(text.c_str(), x, y, fontSize, 1000, fontSize, color);
    }
}
