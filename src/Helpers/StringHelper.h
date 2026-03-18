//
// Created by VrikkaDev on 1.2.2024.
//

#ifndef RLSUDOKU_STRINGHELPER_H
#define RLSUDOKU_STRINGHELPER_H

#include <string>
#include <vector>

class StringHelper {
public:
    static std::vector<std::string> SplitString(const char* text, char splitter);
    static std::string TrimCopy(const std::string& value);
    static std::string ToLowerCopy(const std::string& value);
};


#endif //RLSUDOKU_STRINGHELPER_H
