//
// Created by VrikkaDev on 5.2.2024.
//

#ifndef RLSUDOKU_TIMEHELPER_H
#define RLSUDOKU_TIMEHELPER_H

#include <string>
#include <ctime>

class TimeHelper {
public:
    static std::string GetTimeFormatted(double time, bool showMilliseconds = false, bool forceHours = false);
    static std::string FormatIsoDate(std::time_t raw);
    static std::string FormatDate(std::time_t raw, const std::string& format = "%d.%m.%Y");
    static std::string FormatDateTime(std::time_t raw);
    static std::string FormatIsoDateToDayMonthYear(const std::string& isoDate);
    static std::string FormatHoursFromSeconds(double seconds, int precision = 2);
};


#endif //RLSUDOKU_TIMEHELPER_H
