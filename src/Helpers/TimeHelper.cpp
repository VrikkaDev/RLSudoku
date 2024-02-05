//
// Created by VrikkaDev on 5.2.2024.
//

#include "TimeHelper.h"

const char* TimeHelper::GetTimeFormatted(double time) {
    std::chrono::seconds sec((int)time);
    // Convert seconds into hh:mm:ss
    auto seconds = std::to_string(sec.count() % 60);
    if(seconds.size() == 1) seconds = "0"+seconds;
    auto minutes = std::to_string(std::chrono::duration_cast<std::chrono::minutes>(sec).count() % 60);
    if(minutes.size() == 1) minutes = "0"+minutes;
    auto hours = std::to_string(std::chrono::duration_cast<std::chrono::hours>(sec).count());
    if(hours.size() == 1) hours = "0"+hours;

    std::string tx = hours + ":" + minutes
                     + ":" + seconds;
    return std::move(tx).c_str();
}
