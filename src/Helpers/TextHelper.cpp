//
// Created by VrikkaDev on 30.1.2024.
//

#include "TextHelper.h"

Font Fonts::default_font = {};

void DrawTextB(const char* text, int posX, int posY, int fontSize, Color color){
    DrawTextEx(Fonts::default_font, text, (Vector2){static_cast<float>(posX), static_cast<float>(posY) }, fontSize, 1, color);
}

void DrawTextBC(const char* text, int posX, int posY, int fontSize, int width, int height, Color color){

    // Split the text into lines
    std::vector<std::string> lines = StringHelper::SplitString(text, '\n');

    float maxLineWidth = 0.0f;
    for (const auto& line : lines) {
        Vector2 measuredLine = MeasureTextEx(Fonts::default_font, line.c_str(), fontSize, 1);
        maxLineWidth = fmax(maxLineWidth, measuredLine.x);
    }

    // Calculate the total height of the text
    float totalHeight = static_cast<float>(lines.size()) * (MeasureTextEx(Fonts::default_font, "A", fontSize, 1).y);

    // Calculate the position to center the text in the rectangle
    int textX = posX + (width - maxLineWidth) / 2;
    int textY = posY + (height - totalHeight) / 2;

    // Draw each line of the text
    float currentY = static_cast<float>(textY);
    for (const auto& line : lines) {
        DrawTextEx(Fonts::default_font, line.c_str(), (Vector2){static_cast<float>(textX), currentY}, fontSize, 1, color);
        currentY += MeasureTextEx(Fonts::default_font, "A", fontSize, 1).y;
    }
}

void DrawTextBCL(const char* text, int posX, int posY, int fontSize, int height, Color color){

    // Measure the text size using the specified font and size
    Vector2 textSize = MeasureTextEx(Fonts::default_font, text, fontSize, 1);

    // Calculate the position to center the text in the rectangle
    int textX = posX;
    int textY = posY + (height - textSize.y) / 2;

    DrawTextEx(Fonts::default_font, text, (Vector2){static_cast<float>(textX), static_cast<float>(textY) }, fontSize, 1, color);
}

void DrawTooltipB(const char* text, int fontSize, Color bgColor, Color color){
    const int padding = 5; // Padding around the text
    const int margin = 10; // Margin from the mouse position

    // Split the text into lines
    std::vector<std::string> lines = StringHelper::SplitString(text, '\n');

    // Measure the maximum line width to determine the width of the tooltip
    float maxLineWidth = 0.0f;
    for (const auto& line : lines) {
        Vector2 measuredLine = MeasureTextEx(Fonts::default_font, line.c_str(), fontSize, 1);
        maxLineWidth = fmax(maxLineWidth, measuredLine.x);
    }

    // Calculate the total height of the text
    float totalHeight = static_cast<float>(lines.size()) * (MeasureTextEx(Fonts::default_font, "A", fontSize, 1).y + 2 * padding);

    Vector2 mousePosition = GetMousePosition();

    // Calculate the position of the tooltip to avoid going off-screen
    int tooltipX = static_cast<int>(mousePosition.x) + margin;
    int tooltipY = static_cast<int>(mousePosition.y) + margin;

    // Check if the tooltip exceeds the right edge of the screen
    if (tooltipX + maxLineWidth + 2 * padding > GetScreenWidth()) {
        tooltipX = GetScreenWidth() - static_cast<int>(maxLineWidth + 2 * padding) - margin;
    }

    // Draw the background
    DrawRectangle(tooltipX, tooltipY, static_cast<int>(maxLineWidth + 2 * padding), static_cast<int>(totalHeight), bgColor);

    // Draw each line of the text
    float currentY = static_cast<float>(tooltipY) + padding;
    for (const auto& line : lines) {
        DrawTextB(line.c_str(), tooltipX + padding, static_cast<int>(currentY), fontSize, color);
        currentY += MeasureTextEx(Fonts::default_font, "A", fontSize, 1).y + 2 * padding;
    }
}

