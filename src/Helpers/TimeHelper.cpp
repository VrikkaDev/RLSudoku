//
// Created by VrikkaDev on 5.2.2024.
//

#include "TimeHelper.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace {
std::tm ToLocalTime(std::time_t raw) {
    std::tm localTm{};
#ifdef _WIN32
    localtime_s(&localTm, &raw);
#else
    localtime_r(&raw, &localTm);
#endif
    return localTm;
}
}
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

std::string TimeHelper::FormatIsoDate(std::time_t raw) {
    return FormatDate(raw, "%Y-%m-%d");
}

std::string TimeHelper::FormatDate(std::time_t raw, const std::string& format) {
    if (raw <= 0) {
        return "-";
    }

    const std::tm localTm = ToLocalTime(raw);
    char buffer[64] = {};
    std::strftime(buffer, sizeof(buffer), format.c_str(), &localTm);
    return std::string(buffer);
}

std::string TimeHelper::FormatDateTime(std::time_t raw) {
    return FormatDate(raw, "%d.%m.%Y %H:%M:%S");
}

std::string TimeHelper::FormatIsoDateToDayMonthYear(const std::string& isoDate) {
    if (isoDate.size() < 10) {
        return isoDate;
    }
    // Expected input: YYYY-MM-DD
    return isoDate.substr(8, 2) + "." + isoDate.substr(5, 2) + "." + isoDate.substr(0, 4);
}

std::string TimeHelper::FormatHoursFromSeconds(double seconds, int precision) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(std::max(0, precision)) << (std::max(0.0, seconds) / 3600.0);
    return out.str();
}
