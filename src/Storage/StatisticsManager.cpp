//
// Created for RLSudoku statistics tracking
//

#include "StatisticsManager.h"
#include "Helpers/TimeHelper.h"

StatisticsManager::StatisticsManager() {
    Load();
}

void StatisticsManager::Load() {
    stats = StatisticsData{};

    std::ifstream file(filename);
    if (!file.good()) {
        return;
    }

    try {
        nlohmann::json root = nlohmann::json::parse(file);
        file.close();

        stats.totalGamesStarted = root.value("totalGamesStarted", 0);
        stats.totalGamesCompleted = root.value("totalGamesCompleted", 0);
        stats.totalTimeSeconds = root.value("totalTimeSeconds", 0.0);
        stats.totalMistakes = root.value("totalMistakes", 0);
        stats.totalNumbersPlaced = root.value("totalNumbersPlaced", 0);
        stats.totalNumbersCleared = root.value("totalNumbersCleared", 0);
        stats.runsWithAutoCandidates = root.value("runsWithAutoCandidates", 0);
        stats.runsWithAutoCheck = root.value("runsWithAutoCheck", 0);
        stats.runsWithConflictHighlight = root.value("runsWithConflictHighlight", 0);
        stats.runsWithAssists = root.value("runsWithAssists", 0);
        stats.runsWithoutAssists = root.value("runsWithoutAssists", 0);

        if (root.contains("difficulties") && root["difficulties"].is_array()) {
            auto diffs = root["difficulties"];
            for (size_t i = 0; i < std::min(diffs.size(), stats.difficulties.size()); ++i) {
                const auto& d = diffs[i];
                stats.difficulties[i].gamesStarted = d.value("gamesStarted", 0);
                stats.difficulties[i].gamesCompleted = d.value("gamesCompleted", 0);
                stats.difficulties[i].totalTimeSeconds = d.value("totalTimeSeconds", 0.0);
                stats.difficulties[i].bestTimeSeconds = d.value("bestTimeSeconds", 0.0);
                stats.difficulties[i].numbersPlaced = d.value("numbersPlaced", 0);
                stats.difficulties[i].numbersCleared = d.value("numbersCleared", 0);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to load statistics: " << e.what() << std::endl;
        stats = StatisticsData{};
    }
}

void StatisticsManager::Save() const {
    nlohmann::json root;
    root["totalGamesStarted"] = stats.totalGamesStarted;
    root["totalGamesCompleted"] = stats.totalGamesCompleted;
    root["totalTimeSeconds"] = stats.totalTimeSeconds;
    root["totalMistakes"] = stats.totalMistakes;
    root["totalNumbersPlaced"] = stats.totalNumbersPlaced;
    root["totalNumbersCleared"] = stats.totalNumbersCleared;
    root["runsWithAutoCandidates"] = stats.runsWithAutoCandidates;
    root["runsWithAutoCheck"] = stats.runsWithAutoCheck;
    root["runsWithConflictHighlight"] = stats.runsWithConflictHighlight;
    root["runsWithAssists"] = stats.runsWithAssists;
    root["runsWithoutAssists"] = stats.runsWithoutAssists;

    nlohmann::json diffArray = nlohmann::json::array();
    for (const auto& diff : stats.difficulties) {
        nlohmann::json d;
        d["gamesStarted"] = diff.gamesStarted;
        d["gamesCompleted"] = diff.gamesCompleted;
        d["totalTimeSeconds"] = diff.totalTimeSeconds;
        d["bestTimeSeconds"] = diff.bestTimeSeconds;
        d["numbersPlaced"] = diff.numbersPlaced;
        d["numbersCleared"] = diff.numbersCleared;
        diffArray.push_back(d);
    }
    root["difficulties"] = diffArray;

    std::ofstream file(filename);
    if (!file.good()) {
        std::cerr << "Failed to save statistics to " << filename << std::endl;
        return;
    }

    file << root.dump(4);
    file.close();
}

void StatisticsManager::RecordGameStart(int difficulty) {
    stats.totalGamesStarted++;
    int index = DifficultyToIndex(difficulty);
    stats.difficulties[index].gamesStarted++;
    Save();
}

void StatisticsManager::RecordMistake() {
    stats.totalMistakes++;
    Save();
}

void StatisticsManager::RecordGameCompleted(int difficulty, double completionTimeSeconds,
                                            bool usedAutoCandidates, bool usedAutoCheck, bool usedConflictHighlight) {
    stats.totalGamesCompleted++;
    stats.totalTimeSeconds += std::max(0.0, completionTimeSeconds);

    int index = DifficultyToIndex(difficulty);
    auto& diffStats = stats.difficulties[index];
    diffStats.gamesCompleted++;
    diffStats.totalTimeSeconds += std::max(0.0, completionTimeSeconds);

    if (completionTimeSeconds > 0.0) {
        if (diffStats.bestTimeSeconds <= 0.0 || completionTimeSeconds < diffStats.bestTimeSeconds) {
            diffStats.bestTimeSeconds = completionTimeSeconds;
        }
    }

    bool usedAnyAssist = usedAutoCandidates || usedAutoCheck || usedConflictHighlight;
    if (usedAnyAssist) {
        stats.runsWithAssists++;
    } else {
        stats.runsWithoutAssists++;
    }

    if (usedAutoCandidates) stats.runsWithAutoCandidates++;
    if (usedAutoCheck) stats.runsWithAutoCheck++;
    if (usedConflictHighlight) stats.runsWithConflictHighlight++;

    Save();
}

void StatisticsManager::RecordNumberPlaced(int difficulty) {
    stats.totalNumbersPlaced++;
    int index = DifficultyToIndex(difficulty);
    stats.difficulties[index].numbersPlaced++;
    Save();
}

void StatisticsManager::RecordNumberCleared(int difficulty, bool hadValueBefore) {
    if (!hadValueBefore) {
        return;
    }

    stats.totalNumbersCleared++;
    int index = DifficultyToIndex(difficulty);
    stats.difficulties[index].numbersCleared++;
    Save();
}

int StatisticsManager::DifficultyToIndex(int difficulty) const {
    if (difficulty <= 30) {
        return 0; // Easy
    } else if (difficulty <= 45) {
        return 1; // Medium
    } else if (difficulty <= 55) {
        return 2; // Hard
    }
    return 3; // Very Hard
}
