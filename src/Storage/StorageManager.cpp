//
// Created by VrikkaDev on 1.2.2024.
//

#include "StorageManager.h"
#include "Saveable.h"
#include "GameData.h"
#include "Scenes/Scene.h"

StorageManager::StorageManager() {
}

void StorageManager::Save() {

    // Read config file and parse it into json
    std::ifstream RFile(filename);
    nlohmann::json root = {};
    if(RFile.good()){
        try{
            root = nlohmann::json::parse(RFile);
        }catch (std::exception& exception){
            root = nlohmann::json();
        }
    }
    RFile.close();

    for (const auto& saveable : GameData::currentScene->saveableStack.saveables){
        nlohmann::json sj = saveable->GetJson();
        root[saveable->save_token] = sj;
    }

    // For better readability
    std::string ss = root.dump(4);

    // Create and open a config file
    std::ofstream WFile(filename);
    WFile << ss;
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
    for (const auto& saveable : GameData::currentScene->saveableStack.saveables){
        saveable->Load(root[saveable->save_token]);
    }
}

nlohmann::json StorageManager::GetData(const char* key) {
    if(curr_json.contains(key)){
        return curr_json[key];
    }
    return "";
}
