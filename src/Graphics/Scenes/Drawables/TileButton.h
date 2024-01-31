//
// Created by VrikkaDev on 31.1.2024.
//

#ifndef RLSUDOKU_TILEBUTTON_H
#define RLSUDOKU_TILEBUTTON_H


#include "Graphics/Drawable.h"

class TileButton : public Drawable{
public:
    TileButton();
    TileButton(int tilenum, const char* txt, int correctNum, Rectangle rec);
    void Draw() override;
    void OnStart() override;

    Color color = GRAY;
    Color selectedColor = DARKGRAY;
    Color pressColor = DARKGRAY;
    Color textColor = WHITE;

    int fontSize = 50;

    bool selected = false;

    void DeSelect();

    int tileNumber = 0;
    int correctNum = 0;
    std::vector<int> notes = {};
    const char* text = "Button";
};


#endif //RLSUDOKU_TILEBUTTON_H
