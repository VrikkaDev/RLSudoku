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

    nlohmann::json GetData(const char* key);
private:
    std::string filename = "./game_data.json";
    nlohmann::json curr_json = {};
};


#endif //RLSUDOKU_STORAGEMANAGEMENT_H
