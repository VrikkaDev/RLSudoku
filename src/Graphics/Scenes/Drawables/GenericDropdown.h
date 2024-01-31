//
// Created by VrikkaDev on 31.1.2024.
//

#ifndef RLSUDOKU_GENERICDROPDOWN_H
#define RLSUDOKU_GENERICDROPDOWN_H


#include "Graphics/Drawable.h"
#include "GenericButton.h"

class GenericDropdown : public Drawable{
public:
    GenericDropdown();
    GenericDropdown(std::vector<const char*> txts, Rectangle rec);
    void OnStart() override;
    void Draw() override;

    Color color = GRAY;
    Color hoverColor = DARKGRAY;
    Color pressColor = LIGHTGRAY;
    Color textColor = WHITE;
    Color triangleColor = WHITE;

    int fontSize = 40;

    bool isOpened = false;
    int selectedText = 0;

    float triangleSize = 10;

    std::vector<const char*> texts = {};
    std::vector<GenericButton*> textButtons = {};
};


#endif //RLSUDOKU_GENERICDROPDOWN_H
