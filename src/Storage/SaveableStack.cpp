//
// Created by VrikkaDev on 1.2.2024.
//

#include "SaveableStack.h"

void SaveableStack::AddSaveable(std::unique_ptr<Saveable> saveable) {
    saveables.push_back(std::move(saveable));
}

void SaveableStack::Flush() {
    saveables.clear();
}
