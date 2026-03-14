//
// Created for RLSudoku statistics display
//

#include "StatisticsScene.h"
#include "GameData.h"
#include "MainMenuScene.h"
#include "Storage/StatisticsManager.h"
#include "Helpers/UIHelper.h"
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
    float screenH = GetScreenHeight();

    const float marginX = std::max(12.0f, UIHelper::ScaleX(24.0f));
    const int titleFont = UIHelper::ScaleFont(40);
    const int sectionHeaderFont = UIHelper::ScaleFont(30);
    const int bodyFont = std::max(11, UIHelper::ScaleFont(20));
    const int lineSpacing = std::max(bodyFont + 4, UIHelper::ScaleFont(24));
    const int topY = static_cast<int>(std::max(70.0f, UIHelper::ScaleY(110.0f)));

    auto title = new TextWidget("STATISTICS", static_cast<int>(screenW / 2), static_cast<int>(UIHelper::ScaleY(24.0f)), titleFont, WHITE, true);
    drawableStack->AddDrawable(title);

    auto backBtn = new GenericButton("Back", Rectangle{screenW - UIHelper::ScaleX(220.0f), UIHelper::ScaleY(18.0f), UIHelper::ScaleX(200.0f), UIHelper::ScaleY(48.0f)});
    backBtn->fontSize = UIHelper::ScaleFont(25);
    backBtn->OnClick = [](MouseEvent* event) {
        GameData::SetScene(std::make_unique<MainMenuScene>());
    };
    drawableStack->AddDrawable(backBtn);

    if (!GameData::statisticsManager) {
        auto info = new TextWidget("No statistics available yet.", static_cast<int>(marginX), topY, UIHelper::ScaleFont(24), WHITE, false);
        drawableStack->AddDrawable(info);
        return;
    }

    const auto& stats = GameData::statisticsManager->GetStats();

    float leftColumnX = marginX;
    int currentY = topY;

    auto overallHeader = new TextWidget("Overall Summary", static_cast<int>(leftColumnX), currentY, sectionHeaderFont, YELLOW, false);
    drawableStack->AddDrawable(overallHeader);
    currentY += lineSpacing + UIHelper::ScaleFont(6);

    auto addLine = [this, &currentY, lineSpacing, leftColumnX, bodyFont](const std::string& text) {
        auto line = new TextWidget(text, static_cast<int>(leftColumnX), currentY, bodyFont, WHITE, false);
        drawableStack->AddDrawable(line);
        currentY += lineSpacing;
    };

    double averageTime = (stats.totalGamesCompleted > 0)
        ? stats.totalCompletedTimeSeconds / static_cast<double>(stats.totalGamesCompleted)
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

    // Difficulty section anchored to the right side.
    const float minRightPanelW = UIHelper::ScaleX(300.0f);
    const float maxRightPanelW = UIHelper::ScaleX(620.0f);
    float rightPanelW = std::clamp(screenW * 0.42f, minRightPanelW, maxRightPanelW);
    float rightColumnX = screenW - marginX - rightPanelW;
    int diffHeaderY = topY;

    // Only stack below when side-by-side is genuinely impossible.
   // const bool tinyWidth = screenW < 980.0f;
    const bool overlapsLeft = rightColumnX < (leftColumnX + UIHelper::ScaleX(520.0f));
    /*if (tinyWidth || overlapsLeft) {
        rightColumnX = marginX;
        rightPanelW = screenW - marginX * 2.0f;
        diffHeaderY = currentY + UIHelper::ScaleFont(10);
    }*/

    std::array<const char*, 4> difficultyLabels = {
        "Easy (<=30)",
        "Medium (31-45)",
        "Hard (46-55)",
        "Very Hard (>=56)"
    };

    int diffCols = 1;
    float diffAreaWidth = rightPanelW;
    float cardWidth = diffAreaWidth;
    int cardLineSpacing = std::max(bodyFont + 3, UIHelper::ScaleFont(20));
    int cardHeight = cardLineSpacing * 6;

    for (size_t i = 0; i < stats.difficulties.size(); ++i) {
        const auto& diffStats = stats.difficulties[i];
        int row = static_cast<int>(i) / diffCols;
        int col = static_cast<int>(i) % diffCols;
        int cardX = static_cast<int>(rightColumnX + col * (cardWidth + marginX));
        int cardY = diffHeaderY + row * (cardHeight + UIHelper::ScaleFont(8));

        auto header = new TextWidget(difficultyLabels[i], cardX, cardY, UIHelper::ScaleFont(22), LIGHTGRAY, false);
        drawableStack->AddDrawable(header);
        cardY += cardLineSpacing;

        double diffAverage = (diffStats.gamesCompleted > 0)
            ? diffStats.completedTimeSeconds / static_cast<double>(diffStats.gamesCompleted)
            : 0.0;

        auto line1 = new TextWidget("Start: " + std::to_string(diffStats.gamesStarted) +
                                        "  Done: " + std::to_string(diffStats.gamesCompleted),
                                    cardX, cardY, bodyFont, WHITE, false);
        drawableStack->AddDrawable(line1);
        cardY += cardLineSpacing;

        auto line2 = new TextWidget("Avg: " + FormatTimeOrDash(diffAverage) +
                                        " | Best: " + FormatTimeOrDash(diffStats.bestTimeSeconds),
                                    cardX, cardY, bodyFont, WHITE, false);
        drawableStack->AddDrawable(line2);
        cardY += cardLineSpacing;

        auto line3 = new TextWidget("Place: " + std::to_string(diffStats.numbersPlaced) +
                                        "  Clear: " + std::to_string(diffStats.numbersCleared),
                                    cardX, cardY, bodyFont, WHITE, false);
        drawableStack->AddDrawable(line3);
    }

    // Final fallback hint when height is very tight.
    if (screenH < 760.0f) {
        auto hint = new TextWidget("Tip: Resize window taller for full statistics view.",
            static_cast<int>(marginX), static_cast<int>(screenH - UIHelper::ScaleY(28.0f)),
            UIHelper::ScaleFont(16), LIGHTGRAY, false);
        drawableStack->AddDrawable(hint);
    }
}

void StatisticsScene::OnResize() {
    GameData::SetScene(std::make_unique<StatisticsScene>());
}
