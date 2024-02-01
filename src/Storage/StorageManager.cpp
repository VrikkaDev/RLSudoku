//
// Created by VrikkaDev on 1.2.2024.
//

#include "StorageManager.h"
#include "Saveable.h"

StorageManager::StorageManager() {
}

void StorageManager::Save() {

    // Read config file and parse it into json
    std::ifstream RFile(filename);
    nlohmann::json root = {};
    if(RFile.good()){
        root = nlohmann::json::parse(RFile);
    }
    RFile.close();

    for (const std::unique_ptr<Saveable>& saveable : saveableStack.saveables){
        nlohmann::json sj = saveable->GetJson();
        root[saveable->save_token] = sj;
    }
    // Create and open a config file
    std::ofstream WFile(filename);
    WFile << root;
    WFile.close();
}

void StorageManager::Load() {
    // Read config file and parse it into json
    std::ifstream RFile(filename);
    nlohmann::json root = {};
    if(RFile.good()){
        root = nlohmann::json::parse(RFile);
    }
    RFile.close();

    for (const std::unique_ptr<Saveable>& saveable : saveableStack.saveables){
        saveable->Load(root[saveable->save_token]);
    }
}
