#ifndef RLSUDOKU_ACTIVITYSCENE_H
#define RLSUDOKU_ACTIVITYSCENE_H

#include "Scene.h"

class ActivityScene : public Scene {
public:
    explicit ActivityScene(bool mineOnly = true);

    void Setup() override;
    void OnUpdate() override;
    void OnResize() override;

private:
    bool showMineOnly = false;
    class ActivityHeatmapWidget* heatmap = nullptr;
    class TextWidget* detailHeader = nullptr;
    class TextWidget* detailHours = nullptr;
    class TextWidget* detailGames = nullptr;
    class TextWidget* detailTimes = nullptr;
    class TextWidget* detailAssist = nullptr;

    void UpdateDetails(const struct DailyActivitySummary& day);
};

#endif // RLSUDOKU_ACTIVITYSCENE_H
