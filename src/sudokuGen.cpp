#include "sudokuGen.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>

const std::set<int> VALID = {1,2,3,4,5,6,7,8,9};
bool WIN = false;
std::random_device rd;
std::mt19937 randomGen(rd());

inline bool operator == (const coord &lhs, const coord &rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator < (const coord &lhs, const coord &rhs) {
    return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y);
}

void Cell::setPosition(coord pos) {
    this->pos = pos;
    getNeighbors();
};

void Cell::getNeighbors() {
    // Col and Row Neighbors
    for (auto i = 0; i < pos.x; ++i) {
        neighbors.insert({i, pos.y});
    }
    for (auto j = 0; j < pos.y; ++j) {
        neighbors.insert({pos.x, j});

    }

    // Grid neighbors
    int xGrid = ( pos.x / 3 ) * 3;
    int yGrid = ( pos.y / 3 ) * 3;

    for (int x = xGrid; x <= pos.x; ++x) {
        for (int y = yGrid; (x < pos.x && y < yGrid + 3) || y < pos.y ; ++y) {
            neighbors.insert({x, y});
        }
    }
}

SudokuBoard::SudokuBoard() {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            auto cell = std::make_shared<Cell>(Cell());
            cell->setPosition({i, j});
            cell->setChange(false);
            cells.push_back(cell);
        }
    }
}


void SudokuBoard::doGenSudoku() {
    if ( !genSudoku(0) ) {
        std::cerr << "Unable to generate sudoku puzzle." << '\n';
    }
}

bool SudokuBoard::genSudoku(int index) {
    auto cell = cells.at(index);

    std::set<int> neighborValues = {};

    for ( auto neighbor : cell->neighbors) {
        auto value = this->at(neighbor)->value;
        neighborValues.insert(value);
    }

    std::vector<int> available;
    set_difference(
            VALID.begin(), VALID.end(),
            neighborValues.begin(), neighborValues.end(),
            std::inserter(available, available.begin())
            );
    shuffle(available.begin(), available.end(), randomGen);

    for ( auto nb : available ) {
        cell->value = nb;
        //display();

        if (index == 80 || genSudoku(index + 1)) {
            return true;
        }
    }

    cell->value = 0;
    return false;
}

int posToIndex(const coord &pos) {
    return pos.x * 9 + pos.y;
}

SudokuBoard::SudokuBoard(const char* input) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            auto cell = std::make_shared<Cell>(Cell());

            std::string in(1, input[posToIndex({i, j})]);

            cell->value = stoi(in);
            cell->setPosition({i, j});
            cell->setChange(false);
            cells.push_back(cell);
        }
    }
}

// Get cell by index
std::shared_ptr<Cell> SudokuBoard::at(int index) {
    return this->cells.at(index);
}

// Get cell by position
std::shared_ptr<Cell> SudokuBoard::at(coord pos) {
    auto index = posToIndex(pos);
    return this->at(index);
}

SudokuBoard genBoard(int difficulty) {
    SudokuBoard board{};
    board.difficulty = difficulty;
    board.doGenSudoku();
    board.Puzzled();
    return board;
}


void SudokuBoard::Puzzled() {
    // rand 30 44
    srand(time(NULL));
    int toRemove;

    // Ensure input is within a valid range
    if (difficulty < 1 || difficulty > 100) {
        std::cerr << "Please select valid difficulty\n";
    } else {
        // Calculate the number of cells to remove based on the input
        // For example, you might remove between inputInt and inputInt + 10 cells
        int lowerBound = difficulty;
        int upperBound = difficulty + 10;
        toRemove = (rand() % (upperBound - lowerBound + 1) + lowerBound) / 2;
    }

    for (int i = 0; i < toRemove; ++i) {
        srand(time(NULL));
        int index = rand() % 81;
        int comp = 80 - index;
        while (cells.at(index)->value == 0 || cells.at(comp)->value == 0) {
            index = rand() % 81;
            comp = 80 - index;
        }
        cells.at(index)->setChange(true);
        cells.at(index)->oldValue = cells.at(index)->value;
        cells.at(index)->value = 0;
        cells.at(comp)->setChange(true);
        cells.at(comp)->oldValue = cells.at(comp)->value;
        cells.at(comp)->value = 0;
    }
}

void Cell::setChange(bool chg) {
    this->canChange = chg;
}

bool Cell::getChange() {
    return this->canChange;
}

void SudokuBoard::display() {
    for (int i = 0; i < 81; ++i) {
        auto cell = cells.at(i);
        if (i % 27 == 0) {
            std::cout << "+-------+-------+-------+" << std::endl;
        }
        if (i % 3 == 0) {
            std::cout << "| ";
        }
        if (cell->value != 0) {
            if (cell->getChange()) {
                std::cout << "\x1B[91m" << cell->value << "\033[0m" << " ";
            } else {
                std::cout << cell->value << " ";
            }
        } else {
            std::cout << ". ";
        }
        if (i % 9 == 8) {
            std::cout << "|" << std::endl;
        }
    }
    std::cout << "+-------+-------+-------+" << std::endl;
}

void SudokuBoard::setValue(coord pos, int value) {
    auto cell = cells.at(posToIndex(pos));

    if (cell->getChange()) {
        cell->value = value;
    }
}
void SudokuBoard::setValue(int index, int value) {
    int row = index / 9;
    int column = index % 9;
    setValue({row, column}, value);
}


bool SudokuBoard::completed() {
    int vals = 0;
    WIN = true;
    bool result = true;

    for (auto cell : cells) {
        if (cell->value == 0) {
            vals++;
        }
    }

    for (int i = 0; i < 81; ++i) {
        if (!checkSudoku(i)) {
            result = false;
            break;
        }
    }

    if (!result and vals == 0) {
        WIN = false;
        result = true;
    }


    return result;
}

bool SudokuBoard::checkSudoku(int index) {
    auto cell = cells.at(index);

    if (cell->value == 0) {
        return false;
    }

    std::set<int> neighborValues = {};

    for ( auto neighbor : cell->neighbors) {
        auto value = this->at(neighbor)->value;
        neighborValues.insert(value);
    }

    bool exist = std::find(std::begin(neighborValues), std::end(neighborValues), cell->value) != std::end(neighborValues);

    if (exist) {
        return false;
    }

    return true;
}

bool SudokuBoard::win() {
    return WIN;
}


std::string SudokuBoard::parser() const {
    std::stringstream ostream;

    for(const auto& cell: cells) {
        ostream << cell->value;
    }

    return ostream.str();
}

bool SudokuBoard::operator==(const SudokuBoard& other) const{
    return parser() == other.parser();
}
