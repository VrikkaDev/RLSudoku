//
// Created for RLSudoku statistics tracking
//

#ifndef RLSUDOKU_STATISTICSMANAGER_H
#define RLSUDOKU_STATISTICSMANAGER_H

#include "pch.hxx"
#include <array>
#include <map>

struct DifficultyStats {
    int gamesStarted = 0;
    int gamesCompleted = 0;
    double totalTimeSeconds = 0.0; // Play time for this difficulty (completed + abandoned)
    double completedTimeSeconds = 0.0; // Sum of completion times for finished runs only
    double bestTimeSeconds = 0.0; // 0 indicates no best time yet
    int numbersPlaced = 0;
    int numbersCleared = 0;
};

struct StatisticsData {
    int totalGamesStarted = 0;
    int totalGamesCompleted = 0;
    double totalTimeSeconds = 0.0; // Play time: completed + abandoned run durations
    double totalCompletedTimeSeconds = 0.0; // Sum of completion times for finished runs only
    double totalAppTimeSeconds = 0.0; // Total time app/game loop has been running
    int totalMistakes = 0;
    int totalNumbersPlaced = 0;
    int totalNumbersCleared = 0;

    int runsWithAutoCandidates = 0;
    int runsWithAutoCheck = 0;
    int runsWithConflictHighlight = 0;
    int runsWithAssists = 0;
    int runsWithoutAssists = 0;

    std::array<DifficultyStats, 4> difficulties{}; // 0: Easy, 1: Medium, 2: Hard, 3: Very Hard
};

struct DailyStatistics {
    int gamesStarted = 0;
    int gamesCompleted = 0;
    double gamePlaySeconds = 0.0;
    double appOpenSeconds = 0.0;
    double totalCompletionSeconds = 0.0;
    double bestTimeSeconds = 0.0;
    int assistedRuns = 0;
    int cleanRuns = 0;
};

class StatisticsManager {
public:
    StatisticsManager();

    void RecordGameStart(int difficulty);
    void RecordMistake();
    void RecordGameCompleted(int difficulty, double completionTimeSeconds,
                             bool usedAutoCandidates, bool usedAutoCheck, bool usedConflictHighlight);
    void RecordGameAbandoned(double elapsedSeconds);
    void RecordNumberPlaced(int difficulty);
    void RecordNumberCleared(int difficulty, bool hadValueBefore);
    void RecordAppActiveTime(double deltaSeconds);

    const StatisticsData& GetStats() const { return stats; }
    const std::map<std::string, DailyStatistics>& GetDailyStats() const { return dailyStats; }
    nlohmann::json ExportJson() const;
    bool ImportJson(const nlohmann::json& root);

    void Save() const;
    void Load();

private:
    int DifficultyToIndex(int difficulty) const;
    std::string CurrentDayKey() const;

    StatisticsData stats{};
    std::map<std::string, DailyStatistics> dailyStats{};
    std::string filename = "./stats.json";
};

#endif // RLSUDOKU_STATISTICSMANAGER_H
