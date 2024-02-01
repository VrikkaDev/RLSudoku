//
// Created by VrikkaDev on 1.2.2024.
//

#ifndef RLSUDOKU_SAVEABLE_H
#define RLSUDOKU_SAVEABLE_H

#include "pch.hxx"

class Saveable {
public:
    explicit Saveable(const char* save_tkn);
    virtual ~Saveable() = default;
    virtual nlohmann::json GetJson(){
        return "";
    };
    virtual void Load(const nlohmann::json& data){
    };
    std::string save_token;
};


#endif //RLSUDOKU_SAVEABLE_H
