//
// Created for RLSudoku statistics display
//

#include "StatisticsScene.h"
#include "GameData.h"
#include "MainMenuScene.h"
#include "Storage/StatisticsManager.h"
#include "Scenes/Drawables/GenericButton.h"
#include "Scenes/Drawables/TextWidget.h"
#include "Helpers/TimeHelper.h"
#include <array>
#include <iomanip>
#include <sstream>

StatisticsScene::StatisticsScene() : Scene() {}

static std::string FormatTimeOrDash(double seconds) {
    if (seconds <= 0.0) {
        return "-";
    }

    bool forceHours = seconds >= 3600.0;
    return TimeHelper::GetTimeFormatted(seconds, false, forceHours);
}

void StatisticsScene::Setup() {
    float screenW = GetScreenWidth();

    auto title = new TextWidget("STATISTICS", static_cast<int>(screenW / 2), 30, 40, WHITE, true);
    drawableStack->AddDrawable(title);

    auto backBtn = new GenericButton("Back", Rectangle{screenW - 220, 20, 200, 50});
    backBtn->fontSize = 25;
    backBtn->OnClick = [](MouseEvent* event) {
        GameData::SetScene(std::make_unique<MainMenuScene>());
    };
    drawableStack->AddDrawable(backBtn);

    if (!GameData::statisticsManager) {
        auto info = new TextWidget("No statistics available yet.", 40, 120, 28, WHITE, false);
        drawableStack->AddDrawable(info);
        return;
    }

    const auto& stats = GameData::statisticsManager->GetStats();

    float leftColumnX = 40.0f;
    int lineSpacing = 28;
    int currentY = 110;

    auto overallHeader = new TextWidget("Overall Summary", static_cast<int>(leftColumnX), currentY, 32, YELLOW, false);
    drawableStack->AddDrawable(overallHeader);
    currentY += lineSpacing + 10;

    auto addLine = [this, &currentY, lineSpacing, leftColumnX](const std::string& text) {
        auto line = new TextWidget(text, static_cast<int>(leftColumnX), currentY, 24, WHITE, false);
        drawableStack->AddDrawable(line);
        currentY += lineSpacing;
    };

    double averageTime = (stats.totalGamesCompleted > 0)
        ? stats.totalTimeSeconds / static_cast<double>(stats.totalGamesCompleted)
        : 0.0;

    double completionRate = (stats.totalGamesStarted > 0)
        ? (static_cast<double>(stats.totalGamesCompleted) / static_cast<double>(stats.totalGamesStarted)) * 100.0
        : 0.0;

    std::ostringstream rateStream;
    rateStream << std::fixed << std::setprecision(1) << completionRate;

    double averagePlacements = (stats.totalGamesStarted > 0)
        ? static_cast<double>(stats.totalNumbersPlaced) / static_cast<double>(stats.totalGamesStarted)
        : 0.0;
    double averageClears = (stats.totalGamesStarted > 0)
        ? static_cast<double>(stats.totalNumbersCleared) / static_cast<double>(stats.totalGamesStarted)
        : 0.0;

    std::ostringstream placementsStream;
    placementsStream << std::fixed << std::setprecision(1) << averagePlacements;
    std::ostringstream clearsStream;
    clearsStream << std::fixed << std::setprecision(1) << averageClears;

    addLine("Games Started: " + std::to_string(stats.totalGamesStarted));
    addLine("Games Completed: " + std::to_string(stats.totalGamesCompleted));
    addLine("Completion Rate: " + (stats.totalGamesStarted > 0 ? rateStream.str() + "%" : "-"));
    addLine("Total Play Time: " + FormatTimeOrDash(stats.totalTimeSeconds));
    addLine("Total Time (App Open): " + FormatTimeOrDash(stats.totalAppTimeSeconds));
    addLine("Average Finish Time: " + FormatTimeOrDash(averageTime));
    addLine("Total Mistakes: " + std::to_string(stats.totalMistakes));
    addLine("Numbers Placed: " + std::to_string(stats.totalNumbersPlaced));
    addLine("Numbers Cleared: " + std::to_string(stats.totalNumbersCleared));
    addLine("Avg Placements/Game: " + (stats.totalGamesStarted > 0 ? placementsStream.str() : "-"));
    addLine("Avg Clears/Game: " + (stats.totalGamesStarted > 0 ? clearsStream.str() : "-"));
    addLine("Runs With Assists: " + std::to_string(stats.runsWithAssists));
    addLine("Runs Without Assists: " + std::to_string(stats.runsWithoutAssists));
    addLine("Auto Candidates Used: " + std::to_string(stats.runsWithAutoCandidates));
    addLine("Auto Check Used: " + std::to_string(stats.runsWithAutoCheck));
    addLine("Conflict Highlight Used: " + std::to_string(stats.runsWithConflictHighlight));

    float rightColumnX = screenW / 2.0f + 20.0f;
    int diffHeaderY = 110;

    std::array<const char*, 4> difficultyLabels = {
        "Easy (<=30)",
        "Medium (31-45)",
        "Hard (46-55)",
        "Very Hard (>=56)"
    };

    int diffSpacing = lineSpacing;

    for (size_t i = 0; i < stats.difficulties.size(); ++i) {
        const auto& diffStats = stats.difficulties[i];

        auto header = new TextWidget(difficultyLabels[i], static_cast<int>(rightColumnX), diffHeaderY, 30, LIGHTGRAY, false);
        drawableStack->AddDrawable(header);
        diffHeaderY += diffSpacing + 6;

        double diffAverage = (diffStats.gamesCompleted > 0)
            ? diffStats.totalTimeSeconds / static_cast<double>(diffStats.gamesCompleted)
            : 0.0;

        auto line1 = new TextWidget(
            "Games Started: " + std::to_string(diffStats.gamesStarted),
            static_cast<int>(rightColumnX), diffHeaderY, 22, WHITE, false);
        drawableStack->AddDrawable(line1);
        diffHeaderY += diffSpacing;

        auto line2 = new TextWidget(
            "Games Completed: " + std::to_string(diffStats.gamesCompleted),
            static_cast<int>(rightColumnX), diffHeaderY, 22, WHITE, false);
        drawableStack->AddDrawable(line2);
        diffHeaderY += diffSpacing;

        auto line3 = new TextWidget(
            "Average Time: " + FormatTimeOrDash(diffAverage),
            static_cast<int>(rightColumnX), diffHeaderY, 22, WHITE, false);
        drawableStack->AddDrawable(line3);
        diffHeaderY += diffSpacing;

        auto line4 = new TextWidget(
            "Best Time: " + FormatTimeOrDash(diffStats.bestTimeSeconds),
            static_cast<int>(rightColumnX), diffHeaderY, 22, WHITE, false);
        drawableStack->AddDrawable(line4);
        diffHeaderY += diffSpacing;

        auto line5 = new TextWidget(
            "Numbers Placed: " + std::to_string(diffStats.numbersPlaced),
            static_cast<int>(rightColumnX), diffHeaderY, 22, WHITE, false);
        drawableStack->AddDrawable(line5);
        diffHeaderY += diffSpacing;

        auto line6 = new TextWidget(
            "Numbers Cleared: " + std::to_string(diffStats.numbersCleared),
            static_cast<int>(rightColumnX), diffHeaderY, 22, WHITE, false);
        drawableStack->AddDrawable(line6);
        diffHeaderY += diffSpacing + 12;
    }
}

void StatisticsScene::OnResize() {
    GameData::SetScene(std::make_unique<StatisticsScene>());
}
