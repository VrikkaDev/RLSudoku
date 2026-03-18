#ifndef RLSUDOKU_ACTIVITYHEATMAPWIDGET_H
#define RLSUDOKU_ACTIVITYHEATMAPWIDGET_H

#include "Graphics/Drawable.h"
#include "Storage/RemoteSyncManager.h"

class ActivityHeatmapWidget : public Drawable {
public:
    explicit ActivityHeatmapWidget(const Rectangle& rec);

    void Draw() override;
    void OnUpdate() override;

    void SetDays(const std::vector<DailyActivitySummary>& value);
    void SetSelectedIndex(int index);

    std::function<void(int, const DailyActivitySummary&)> OnDaySelected;

private:
    std::vector<DailyActivitySummary> days;
    int hoveredIndex = -1;
    int selectedIndex = -1;

    [[nodiscard]] int GetCellIndexAt(Vector2 point) const;
    [[nodiscard]] int GetColumns() const;
    [[nodiscard]] int GetRows() const;
    [[nodiscard]] float GetCellSize() const;
    [[nodiscard]] float GetCellGap() const;
    [[nodiscard]] Color GetCellColor(const DailyActivitySummary& day, double maxSeconds) const;
};

#endif // RLSUDOKU_ACTIVITYHEATMAPWIDGET_H
