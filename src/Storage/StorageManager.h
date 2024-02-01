//
// Created by VrikkaDev on 1.2.2024.
//

#ifndef RLSUDOKU_STORAGEMANAGEMENT_H
#define RLSUDOKU_STORAGEMANAGEMENT_H


#include "SaveableStack.h"

class StorageManager {
public:
    StorageManager();

    void Save();
    void Load();

    SaveableStack saveableStack = SaveableStack();
private:
    std::string filename = "./options_save.json";
};


#endif //RLSUDOKU_STORAGEMANAGEMENT_H
