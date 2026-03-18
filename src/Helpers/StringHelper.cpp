//
// Created by VrikkaDev on 1.2.2024.
//

#include "StringHelper.h"

#include <algorithm>
#include <cctype>
#include <sstream>

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

std::string StringHelper::TrimCopy(const std::string& value) {
    size_t start = 0;
    size_t end = value.size();

    while (start < end && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }

    return value.substr(start, end - start);
}

std::string StringHelper::ToLowerCopy(const std::string& value) {
    std::string lowered = value;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return lowered;
}
