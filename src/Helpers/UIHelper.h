// Shared UI utilities for resolution scaling and theme lookups.

#ifndef RLSUDOKU_UIHELPER_H
#define RLSUDOKU_UIHELPER_H

#include "GameData.h"
#include "Storage/StorageManager.h"
#include <algorithm>
#include <cmath>

namespace UIHelper {

constexpr float kDesignWidth = 1920.0f;
constexpr float kDesignHeight = 1080.0f;

inline float ScaleX(float value) {
    return value * (static_cast<float>(GetScreenWidth()) / kDesignWidth);
}

inline float ScaleY(float value) {
    return value * (static_cast<float>(GetScreenHeight()) / kDesignHeight);
}

inline int ScaleFont(int value, int minSize = 12) {
    const float scale = std::min(static_cast<float>(GetScreenWidth()) / kDesignWidth,
                                 static_cast<float>(GetScreenHeight()) / kDesignHeight);
    return std::max(minSize, static_cast<int>(std::round(static_cast<float>(value) * scale)));
}

inline bool IsDarkModeEnabled() {
    if (!GameData::storageManager) {
        return false;
    }

    nlohmann::json mode = GameData::storageManager->GetData("options_toggle_darkmode");
    return mode.contains("value") && mode["value"].is_boolean() && mode["value"];
}

} // namespace UIHelper

#endif // RLSUDOKU_UIHELPER_H
