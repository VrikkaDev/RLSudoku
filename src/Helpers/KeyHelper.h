//
// Created by VrikkaDev on 1.2.2024.
//

#ifndef RLSUDOKU_KEYHELPER_H
#define RLSUDOKU_KEYHELPER_H


class KeyHelper {
public:
    // Function to map a numeric input to its corresponding number key
    // Returns the number associated with the key or -1 if the key is not mapped
    static int GetNumberFromKeyNum(int key) {

        auto it = keyNumberMap.find(key);
        return (it != keyNumberMap.end()) ? it->second : -1; // -1 indicates key not found
    }
private:
    static std::map<int, int> keyNumberMap;
};


#endif //RLSUDOKU_KEYHELPER_H
