//
// Created by VrikkaDev on 1.2.2024.
//

#ifndef RLSUDOKU_SUDOKUHELPER_H
#define RLSUDOKU_SUDOKUHELPER_H

#include "pch.hxx"

class SudokuHelper {
public:
    static sudoku::GeneratorResult GenerateSudoku(sudoku::PuzzleDifficulty difficulty, sudoku::Board &generatedBoard);
};


#endif //RLSUDOKU_SUDOKUHELPER_H
