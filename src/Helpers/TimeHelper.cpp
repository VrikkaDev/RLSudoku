//
// Created by VrikkaDev on 5.2.2024.
//

#include "TimeHelper.h"

#include <cmath>
#include <iomanip>
#include <sstream>

std::string TimeHelper::GetTimeFormatted(double time, bool showMilliseconds, bool forceHours) {
    if (std::isnan(time) || std::isinf(time) || time < 0.0) {
        time = 0.0;
    }

    int totalSeconds = static_cast<int>(std::floor(time));
    double fractional = time - static_cast<double>(totalSeconds);

    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    int milliseconds = static_cast<int>(std::round(fractional * 1000.0));

    if (milliseconds == 1000) {
        milliseconds = 0;
        ++seconds;
        if (seconds == 60) {
            seconds = 0;
            ++minutes;
            if (minutes == 60) {
                minutes = 0;
                ++hours;
            }
        }
    }

    bool includeHours = forceHours || hours > 0;
    int displayMinutes = includeHours ? minutes : totalSeconds / 60;

    std::ostringstream oss;
    oss << std::setfill('0');

    if (includeHours) {
        oss << hours << ':' << std::setw(2) << minutes << ':' << std::setw(2) << seconds;
    } else {
        oss << displayMinutes << ':' << std::setw(2) << seconds;
    }

    if (showMilliseconds) {
        oss << '.' << std::setw(3) << milliseconds;
    }

    return oss.str();
}
