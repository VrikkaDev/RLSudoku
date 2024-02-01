//
// Created by VrikkaDev on 1.2.2024.
//

#include "StringHelper.h"

std::vector<std::string> StringHelper::SplitString(const char* text, char splitter) {
    // Split string :D
    std::vector<std::string> tokens;
    std::istringstream tokenStream(text);
    std::string token;
    while (std::getline(tokenStream, token, splitter)) {
        tokens.push_back(token);
    }
    return tokens;
}
