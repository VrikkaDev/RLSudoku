//
// Created by VrikkaDev on 31.1.2024.
//

#ifndef RLSUDOKU_TILEGRID_H
#define RLSUDOKU_TILEGRID_H


#include "Graphics/Drawable.h"
#include "Storage/Saveable.h"

class TileGrid : public Drawable{
public:
    TileGrid();
    explicit TileGrid(SudokuBoard* brd, SudokuBoard* orgBrd, SudokuBoard* solved, Rectangle rec);
    void Draw() override;
    void OnStart() override;

    void CheckIfFinished();

    void SetTile(int tilenumber, int value);
    void SelectTile(int tilenumber);

    int selectedTile = -1;// Selected tile -1 is none

    bool isFinished = false;

    // This could be used for more things than only the 9 detection. would probably be more efficient.
    std::map<int,int> tileValues{};// {tilenumber, value}

    Color color = DARKGRAY;
private:
    SudokuBoard* board;
    SudokuBoard* solvedBoard;
    SudokuBoard* orgBoard;

    bool isPaused = false;
};


#endif //RLSUDOKU_TILEGRID_H
