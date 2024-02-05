
#ifndef RLSUDOKU_SUDOKUGEN_H
#define RLSUDOKU_SUDOKUGEN_H


#include <set>
#include <deque>
#include <memory>
#include <sstream>

// MODFIFIED https://github.com/Zetsuban/SupSudoku generator


struct coord {
    int x = 0;
    int y = 0;
};

class Cell {
public:
    coord pos;
    std::set<coord> neighbors{};
    int value = 0;
    int oldValue = 0;

    void setChange( bool chg );
    bool getChange();
    void setPosition( coord pos );
    void getNeighbors();
    Cell() {}

private:
    bool canChange;
};

using cellQueu = std::deque<std::shared_ptr<Cell>>;

class SudokuBoard {
    cellQueu cells;

public:
    void doGenSudoku();
    void Puzzled();
    void display();
    bool completed();
    void setValue(coord pos, int value);
    void setValue(int index, int value);
    bool win();
    [[nodiscard]] std::string parser() const;
    SudokuBoard();
    explicit SudokuBoard(const char* input);

    int difficulty;

    std::shared_ptr<Cell> at( int index );
    std::shared_ptr<Cell> at( coord pos );
    bool operator ==(const SudokuBoard& other) const;

    bool checkSudoku( int index );
private:
    bool genSudoku( int index );
};

SudokuBoard genBoard(int difficulty);

#endif //RLSUDOKU_SUDOKUGEN_H