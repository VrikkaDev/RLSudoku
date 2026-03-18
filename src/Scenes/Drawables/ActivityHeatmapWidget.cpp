#include "ActivityHeatmapWidget.h"

#include "Helpers/TimeHelper.h"
#include "Helpers/TextHelper.h"
#include "Helpers/UIHelper.h"

#include <algorithm>

ActivityHeatmapWidget::ActivityHeatmapWidget(const Rectangle& rec) : Drawable() {
    x = static_cast<int>(rec.x);
    y = static_cast<int>(rec.y);
    width = static_cast<int>(rec.width);
    height = static_cast<int>(rec.height);
    OnDaySelected = [](int, const DailyActivitySummary&) {};
}

void ActivityHeatmapWidget::SetDays(const std::vector<DailyActivitySummary>& value) {
    days = value;
    hoveredIndex = -1;
    if (selectedIndex >= static_cast<int>(days.size())) {
        selectedIndex = -1;
    }
}

void ActivityHeatmapWidget::SetSelectedIndex(int index) {
    if (index < 0 || index >= static_cast<int>(days.size())) {
        return;
    }
    selectedIndex = index;
}

void ActivityHeatmapWidget::OnUpdate() {
    hoveredIndex = GetCellIndexAt(GetMousePosition());

    if (hoveredIndex >= 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        selectedIndex = hoveredIndex;
        if (OnDaySelected) {
            OnDaySelected(selectedIndex, days[selectedIndex]);
        }
    }
}

void ActivityHeatmapWidget::Draw() {
    const bool darkMode = UIHelper::IsDarkModeEnabled();
    const Color panelColor = darkMode ? CLITERAL(Color){33, 33, 39, 255} : CLITERAL(Color){240, 240, 240, 255};
    const Color borderColor = darkMode ? CLITERAL(Color){74, 74, 86, 255} : CLITERAL(Color){170, 170, 170, 255};

    DrawRectangle(x, y, width, height, panelColor);
    DrawRectangleLines(x, y, width, height, borderColor);

    if (days.empty()) {
        DrawTextBCL("No daily data", x + 8, y + height / 2 - 10, UIHelper::ScaleFont(16), 20, darkMode ? LIGHTGRAY : DARKGRAY);
        return;
    }

    double maxSeconds = 0.0;
    for (const auto& day : days) {
        maxSeconds = std::max(maxSeconds, day.totalCompletionSeconds);
    }

    const int rows = GetRows();
    const int cols = GetColumns();
    const float cell = GetCellSize();
    const float gap = GetCellGap();

    const float totalGridW = cols * cell + (cols - 1) * gap;
    const float totalGridH = rows * cell + (rows - 1) * gap;

    const float startX = static_cast<float>(x) + std::max(6.0f, (static_cast<float>(width) - totalGridW) * 0.5f);
    const float startY = static_cast<float>(y) + std::max(6.0f, (static_cast<float>(height) - totalGridH) * 0.5f);

    for (size_t i = 0; i < days.size(); ++i) {
        const int row = static_cast<int>(i % rows);
        const int col = static_cast<int>(i / rows);
        const float cellX = startX + col * (cell + gap);
        const float cellY = startY + row * (cell + gap);

        const Rectangle r{cellX, cellY, cell, cell};
        const bool isHovered = static_cast<int>(i) == hoveredIndex;
        const bool isSelected = static_cast<int>(i) == selectedIndex;

        DrawRectangleRec(r, GetCellColor(days[i], maxSeconds));
        if (isHovered || isSelected) {
            DrawRectangleLinesEx(r, isSelected ? 2.0f : 1.0f, isSelected ? YELLOW : WHITE);
        }
    }

    if (hoveredIndex >= 0 && hoveredIndex < static_cast<int>(days.size())) {
        const auto& day = days[hoveredIndex];
        std::string tooltip = TimeHelper::FormatIsoDateToDayMonthYear(day.date)
            + "\nGame: " + TimeHelper::FormatHoursFromSeconds(day.totalCompletionSeconds) + " h"
            + "\nApp: " + TimeHelper::FormatHoursFromSeconds(day.appOpenSeconds) + " h"
            + "\nStarted / Completed: " + std::to_string(day.gamesStarted) + " / " + std::to_string(day.gamesCompleted);
        DrawTooltipB(tooltip.c_str(), UIHelper::ScaleFont(14), CLITERAL(Color){20, 20, 20, 230}, WHITE);
    }
}

int ActivityHeatmapWidget::GetCellIndexAt(Vector2 point) const {
    if (days.empty()) {
        return -1;
    }

    const int rows = GetRows();
    const int cols = GetColumns();
    const float cell = GetCellSize();
    const float gap = GetCellGap();

    const float totalGridW = cols * cell + (cols - 1) * gap;
    const float totalGridH = rows * cell + (rows - 1) * gap;

    const float startX = static_cast<float>(x) + std::max(6.0f, (static_cast<float>(width) - totalGridW) * 0.5f);
    const float startY = static_cast<float>(y) + std::max(6.0f, (static_cast<float>(height) - totalGridH) * 0.5f);

    for (size_t i = 0; i < days.size(); ++i) {
        const int row = static_cast<int>(i % rows);
        const int col = static_cast<int>(i / rows);
        Rectangle r{
            startX + col * (cell + gap),
            startY + row * (cell + gap),
            cell,
            cell,
        };
        if (CheckCollisionPointRec(point, r)) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

int ActivityHeatmapWidget::GetColumns() const {
    const int rows = GetRows();
    return std::max(1, static_cast<int>((days.size() + rows - 1) / rows));
}

int ActivityHeatmapWidget::GetRows() const {
    return 7;
}

float ActivityHeatmapWidget::GetCellSize() const {
    const int rows = GetRows();
    const int cols = GetColumns();
    const float gap = GetCellGap();

    float byWidth = (static_cast<float>(width) - std::max(0, cols - 1) * gap - 12.0f) / static_cast<float>(std::max(1, cols));
    float byHeight = (static_cast<float>(height) - std::max(0, rows - 1) * gap - 12.0f) / static_cast<float>(rows);
    float cell = std::floor(std::min(byWidth, byHeight));
    return std::max(4.0f, cell);
}

float ActivityHeatmapWidget::GetCellGap() const {
    return 3.0f;
}

Color ActivityHeatmapWidget::GetCellColor(const DailyActivitySummary& day, double maxSeconds) const {
    const bool darkMode = UIHelper::IsDarkModeEnabled();
    const Color off = darkMode ? CLITERAL(Color){48, 48, 56, 255} : CLITERAL(Color){225, 225, 225, 255};

    if (day.totalCompletionSeconds <= 0.0 || maxSeconds <= 0.0) {
        return off;
    }

    const double ratio = day.totalCompletionSeconds / maxSeconds;
    if (ratio < 0.25) {
        return CLITERAL(Color){14, 104, 57, 255};
    }
    if (ratio < 0.5) {
        return CLITERAL(Color){38, 166, 65, 255};
    }
    if (ratio < 0.75) {
        return CLITERAL(Color){57, 211, 83, 255};
    }
    return CLITERAL(Color){95, 255, 120, 255};
}
