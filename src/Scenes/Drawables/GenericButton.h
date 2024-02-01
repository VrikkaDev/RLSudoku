//
// Created by VrikkaDev on 30.1.2024.
//

#ifndef RLSUDOKU_GENERICBUTTON_H
#define RLSUDOKU_GENERICBUTTON_H


#include "Graphics/Drawable.h"

class GenericButton : public Drawable{
public:
    GenericButton();
    GenericButton(const char* txt, Rectangle rec);
    void Draw() override;

    Color color = GRAY;
    Color hoverColor = DARKGRAY;
    Color pressColor = LIGHTGRAY;
    Color textColor = WHITE;

    int fontSize = 50;

    const char* text = "Button";
};


#endif //RLSUDOKU_GENERICBUTTON_H
