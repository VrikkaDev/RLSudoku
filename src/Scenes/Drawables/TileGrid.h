//
// Created by VrikkaDev on 31.1.2024.
//

#ifndef RLSUDOKU_TILEGRID_H
#define RLSUDOKU_TILEGRID_H


#include "Graphics/Drawable.h"

class TileGrid : public Drawable{
public:
    TileGrid();
    explicit TileGrid(sudoku::Board* brd, sudoku::Solver* slvr, Rectangle rec);
    void Draw() override;
    void OnStart() override;

    void SelectTile(int tilenumber);

    int selectedTile = -1;// Selected tile -1 is none

    sudoku::Solver* solver = nullptr;
    sudoku::Board* board = nullptr;
    sudoku::Board* solvedBoard = nullptr;

    Color color = DARKGRAY;
};


#endif //RLSUDOKU_TILEGRID_H
