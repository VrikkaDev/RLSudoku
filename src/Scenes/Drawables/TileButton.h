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

    Color color = LIGHTGRAY;
    Color selectedColor = DARKGRAY;

    Color gridlineColor = GRAY;

    Color textColor = WHITE; // Text color when selected
    Color textColor2 = DARKGRAY; // Text color when not selected
    Color textColor3 = BLACK; // Text color for permanent big numbers
    Color wrongNumColor = RED;

    int fontSize = 50;

    bool showIsWrong = false;// If true renders the text with the wrongNumColor
    bool showConflicts = false;// If true renders red ball

    bool selected = false;
    bool permanent = false; // When permanent color is textColor´3 and cannot be changed

    bool inGridLine = false;

    void DeSelect();
    void SetText(const std::string& str);
    void CheckForConflicts();

    int tileNumber = 0;
    int correctNum = 0;

    void addConflict(int tile);
    void removeConflict(int tile);
    std::vector<int> conflicts = {};
    std::vector<int> notes = {};
    std::string text;
};


#endif //RLSUDOKU_TILEBUTTON_H
