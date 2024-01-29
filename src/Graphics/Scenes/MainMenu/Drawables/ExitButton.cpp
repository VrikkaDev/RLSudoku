//
// Created by VrikkaDev on 29.1.2024.
//

#include "ExitButton.h"

ExitButton::ExitButton() : Drawable(){
    width = 50;
    height = 50;

    x = GetScreenWidth() - width - 10;
    y = 10;
}

void ExitButton::Draw() {
    DrawRectangle(x, y, width, height, GRAY);
    // Measure the text width and height
    int textWidth = MeasureText("X", height);  // Measure the width based on the text height
    int textHeight = height;  // Use the same height as the rectangle

    // Calculate the position to center the text in the rectangle
    int textX = x + (width - textWidth) / 2;
    int textY = y + (height - textHeight) / 2;

    // Draw the text in the middle of the rectangle
    DrawTextB("X", textX, textY, textHeight, WHITE);
}

void ExitButton::OnClick(MouseEvent* event) {
    std::cout<<"MouseEvent( EventType="<< event->EventType <<
             ", MousePosition=(x:" << event->MousePosition.x <<
             ", y:" << event->MousePosition.y << "))" << std::endl;
}
