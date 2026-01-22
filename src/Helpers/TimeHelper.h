//
// Created by VrikkaDev on 5.2.2024.
//

#ifndef RLSUDOKU_TIMEHELPER_H
#define RLSUDOKU_TIMEHELPER_H

#include <string>

class TimeHelper {
public:
    static std::string GetTimeFormatted(double time, bool showMilliseconds = false, bool forceHours = false);
};


#endif //RLSUDOKU_TIMEHELPER_H
