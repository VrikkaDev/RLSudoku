#include "ActivityScene.h"

#include "GameData.h"
#include "MainMenuScene.h"
#include "Helpers/TimeHelper.h"
#include "Scenes/Drawables/GenericButton.h"
#include "Scenes/Drawables/TextWidget.h"
#include "Scenes/Drawables/ActivityHeatmapWidget.h"
#include "Storage/RemoteSyncManager.h"
#include "Storage/StatisticsManager.h"
#include "Helpers/UIHelper.h"

#include <sstream>

ActivityScene::ActivityScene(bool mineOnly) : Scene(), showMineOnly(mineOnly) {}

void ActivityScene::Setup() {
    const float screenW = static_cast<float>(GetScreenWidth());
    const float screenH = static_cast<float>(GetScreenHeight());
    const float marginX = std::max(12.0f, UIHelper::ScaleX(20.0f));

    auto* title = new TextWidget("PLAY ACTIVITY", static_cast<int>(screenW / 2), static_cast<int>(UIHelper::ScaleY(20.0f)), UIHelper::ScaleFont(40), WHITE, true);
    drawableStack->AddDrawable(title);

    auto* backBtn = new GenericButton("Back", Rectangle{screenW - UIHelper::ScaleX(220.0f), UIHelper::ScaleY(16.0f), UIHelper::ScaleX(200.0f), UIHelper::ScaleY(48.0f)});
    backBtn->fontSize = UIHelper::ScaleFont(24);
    backBtn->OnClick = [](MouseEvent* event) {
        GameData::SetScene(std::make_unique<MainMenuScene>());
    };
    drawableStack->AddDrawable(backBtn);

    auto* refreshBtn = new GenericButton("Refresh", Rectangle{screenW - UIHelper::ScaleX(440.0f), UIHelper::ScaleY(16.0f), UIHelper::ScaleX(200.0f), UIHelper::ScaleY(48.0f)});
    refreshBtn->fontSize = UIHelper::ScaleFont(24);
    refreshBtn->OnClick = [this](MouseEvent* event) {
        GameData::SetScene(std::make_unique<ActivityScene>(showMineOnly));
    };
    drawableStack->AddDrawable(refreshBtn);

    auto* scopeBtn = new GenericButton(showMineOnly ? "Scope: Mine" : "Scope: Global", Rectangle{screenW - UIHelper::ScaleX(660.0f), UIHelper::ScaleY(16.0f), UIHelper::ScaleX(200.0f), UIHelper::ScaleY(48.0f)});
    scopeBtn->fontSize = UIHelper::ScaleFont(22);
    scopeBtn->OnClick = [this](MouseEvent* event) {
        GameData::SetScene(std::make_unique<ActivityScene>(!showMineOnly));
    };
    drawableStack->AddDrawable(scopeBtn);

    std::vector<DailyActivitySummary> summaries;
    bool fetchOk = false;
    if (GameData::remoteSyncManager) {
        fetchOk = GameData::remoteSyncManager->RefreshDailyActivitySummaryNow(365, showMineOnly);
        summaries = GameData::remoteSyncManager->GetCachedDailyActivitySummary();
    }

    const int subtitleFont = UIHelper::ScaleFont(18);
    const int bodyFont = std::max(12, UIHelper::ScaleFont(18));

    std::ostringstream summary;
    summary << "Heatmap: " << (showMineOnly ? "Mine" : "Global")
            << " | Days with play: " << summaries.size()
            << " | Last fetch: " << (fetchOk ? "ok" : "cached/offline");
    auto* subtitle = new TextWidget(summary.str(), static_cast<int>(marginX), static_cast<int>(UIHelper::ScaleY(74.0f)), subtitleFont, LIGHTGRAY, false);
    drawableStack->AddDrawable(subtitle);

    std::map<std::string, DailyActivitySummary> byDate;
    for (const auto& day : summaries) {
        byDate[day.date] = day;
    }

    std::vector<DailyActivitySummary> days;
    days.reserve(364);
    std::time_t now = std::time(nullptr);
    const std::time_t dayStep = 24 * 60 * 60;
    const std::time_t start = now - dayStep * 363;
    for (int i = 0; i < 364; ++i) {
        std::time_t dayTs = start + dayStep * i;
        std::string key = TimeHelper::FormatIsoDate(dayTs);

        auto found = byDate.find(key);
        if (found != byDate.end()) {
            days.push_back(found->second);
        } else {
            DailyActivitySummary empty;
            empty.date = key;
            days.push_back(empty);
        }
    }

    if (showMineOnly && GameData::statisticsManager) {
        const auto& localDaily = GameData::statisticsManager->GetDailyStats();
        for (auto& day : days) {
            auto localIt = localDaily.find(day.date);
            if (localIt == localDaily.end()) {
                continue;
            }

            const DailyStatistics& local = localIt->second;
            day.gamesStarted = local.gamesStarted;
            day.totalCompletionSeconds = local.gamePlaySeconds;
            day.appOpenSeconds = local.appOpenSeconds;
            day.gamesCompleted = local.gamesCompleted;
            day.assistedRuns = local.assistedRuns;
            day.cleanRuns = local.cleanRuns;
            day.bestTimeSeconds = local.bestTimeSeconds;
            day.averageCompletionSeconds = (local.gamesCompleted > 0)
                ? (local.totalCompletionSeconds / static_cast<double>(local.gamesCompleted))
                : 0.0;
        }
    }

    const float heatmapX = marginX;
    const float heatmapY = UIHelper::ScaleY(112.0f);
    const float heatmapW = std::min(screenW - marginX * 2.0f, UIHelper::ScaleX(1260.0f));
    const float heatmapH = std::min(UIHelper::ScaleY(260.0f), screenH * 0.42f);

    heatmap = new ActivityHeatmapWidget(Rectangle{heatmapX, heatmapY, heatmapW, heatmapH});
    heatmap->SetDays(days);
    heatmap->OnDaySelected = [this](int index, const DailyActivitySummary& day) {
        UpdateDetails(day);
    };
    drawableStack->AddDrawable(heatmap);

    detailHeader = new TextWidget("Select a day to inspect details", static_cast<int>(marginX), static_cast<int>(heatmapY + heatmapH + UIHelper::ScaleY(16.0f)), bodyFont, YELLOW, false);
    drawableStack->AddDrawable(detailHeader);

    detailHours = new TextWidget("Game Play / App Open: -", static_cast<int>(marginX), detailHeader->y + UIHelper::ScaleFont(30), bodyFont, WHITE, false);
    drawableStack->AddDrawable(detailHours);

    detailGames = new TextWidget("Games Started / Completed: -", static_cast<int>(marginX), detailHours->y + UIHelper::ScaleFont(26), bodyFont, WHITE, false);
    drawableStack->AddDrawable(detailGames);

    detailTimes = new TextWidget("Best / Avg Time: -", static_cast<int>(marginX), detailGames->y + UIHelper::ScaleFont(26), bodyFont, WHITE, false);
    drawableStack->AddDrawable(detailTimes);

    detailAssist = new TextWidget("Assist / Clean Runs: -", static_cast<int>(marginX), detailTimes->y + UIHelper::ScaleFont(26), bodyFont, WHITE, false);
    drawableStack->AddDrawable(detailAssist);

    int selected = -1;
    for (int i = static_cast<int>(days.size()) - 1; i >= 0; --i) {
        if (days[i].gamesCompleted > 0 || days[i].gamesStarted > 0) {
            selected = i;
            break;
        }
    }
    if (selected >= 0 && heatmap) {
        heatmap->SetSelectedIndex(selected);
        UpdateDetails(days[selected]);
    }
}

void ActivityScene::OnUpdate() {
    Scene::OnUpdate();
}

void ActivityScene::OnResize() {
    GameData::SetScene(std::make_unique<ActivityScene>(showMineOnly));
}

void ActivityScene::UpdateDetails(const DailyActivitySummary& day) {
    if (!detailHeader || !detailHours || !detailGames || !detailTimes || !detailAssist) {
        return;
    }

    detailHeader->text = "Day: " + TimeHelper::FormatIsoDateToDayMonthYear(day.date);

    detailHours->text = "Game Play / App Open: "
        + TimeHelper::FormatHoursFromSeconds(day.totalCompletionSeconds) + " h / "
        + TimeHelper::FormatHoursFromSeconds(day.appOpenSeconds) + " h";

    detailGames->text = "Games Started / Completed: " + std::to_string(day.gamesStarted)
        + " / " + std::to_string(day.gamesCompleted);

    std::string bestText = day.bestTimeSeconds > 0.0 ? TimeHelper::GetTimeFormatted(day.bestTimeSeconds) : std::string("-");
    std::string avgText = day.averageCompletionSeconds > 0.0 ? TimeHelper::GetTimeFormatted(day.averageCompletionSeconds) : std::string("-");
    detailTimes->text = "Best / Avg Time: " + bestText + " / " + avgText;

    detailAssist->text = "Assist / Clean Runs: " + std::to_string(day.assistedRuns) + " / " + std::to_string(day.cleanRuns);
}
