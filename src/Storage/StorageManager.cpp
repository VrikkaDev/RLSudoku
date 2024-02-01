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

    curr_json = root;
}

void StorageManager::Load() {
    // Read config file and parse it into json
    std::ifstream RFile(filename);
    nlohmann::json root = {};
    if(RFile.good()){
        root = nlohmann::json::parse(RFile);
    }
    RFile.close();

    curr_json = root;

    // Load all saveables
    for (const std::unique_ptr<Saveable>& saveable : saveableStack.saveables){
        saveable->Load(root[saveable->save_token]);
    }
}

nlohmann::json StorageManager::GetData(const char* key) {
    if(curr_json.contains(key)){
        return curr_json[key];
    }
    return "";
}
