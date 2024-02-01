//
// Created by VrikkaDev on 1.2.2024.
//

#ifndef RLSUDOKU_NUMBERBUTTONS_H
#define RLSUDOKU_NUMBERBUTTONS_H


#include "Graphics/Drawable.h"

class NumberButtons : public Drawable{
public:
    NumberButtons();
    explicit NumberButtons(Rectangle rec);
    void OnStart() override;
    void Draw() override;

    Color color = DARKGRAY;
};


#endif //RLSUDOKU_NUMBERBUTTONS_H
