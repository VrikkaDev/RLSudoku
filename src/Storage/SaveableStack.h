//
// Created by VrikkaDev on 1.2.2024.
//

#ifndef RLSUDOKU_SAVEABLESTACK_H
#define RLSUDOKU_SAVEABLESTACK_H

#include "Saveable.h"

class SaveableStack {
public:
    SaveableStack() = default;

    void AddSaveable(std::unique_ptr<Saveable> saveable);
    std::vector<std::unique_ptr<Saveable>> saveables = {};
};


#endif //RLSUDOKU_SAVEABLESTACK_H
