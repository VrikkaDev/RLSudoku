//
// Created for RLSudoku statistics tracking
//

#include "StatisticsManager.h"
#include "Helpers/TimeHelper.h"

StatisticsManager::StatisticsManager() {
    Load();
}

void StatisticsManager::Load() {
    // Server-first: runtime stats are loaded via remote sync, local file is backup-only.
    stats = StatisticsData{};
}

nlohmann::json StatisticsManager::ExportJson() const {
    nlohmann::json root;
    root["totalGamesStarted"] = stats.totalGamesStarted;
    root["totalGamesCompleted"] = stats.totalGamesCompleted;
    root["totalTimeSeconds"] = stats.totalTimeSeconds;
    root["totalCompletedTimeSeconds"] = stats.totalCompletedTimeSeconds;
    root["totalAppTimeSeconds"] = stats.totalAppTimeSeconds;
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
        d["completedTimeSeconds"] = diff.completedTimeSeconds;
        d["bestTimeSeconds"] = diff.bestTimeSeconds;
        d["numbersPlaced"] = diff.numbersPlaced;
        d["numbersCleared"] = diff.numbersCleared;
        diffArray.push_back(d);
    }
    root["difficulties"] = diffArray;

    return root;
}

bool StatisticsManager::ImportJson(const nlohmann::json& root) {
    try {
        StatisticsData imported{};

        imported.totalGamesStarted = root.value("totalGamesStarted", 0);
        imported.totalGamesCompleted = root.value("totalGamesCompleted", 0);
        imported.totalTimeSeconds = root.value("totalTimeSeconds", 0.0);
        imported.totalCompletedTimeSeconds = root.value("totalCompletedTimeSeconds", imported.totalTimeSeconds);
        imported.totalAppTimeSeconds = root.value("totalAppTimeSeconds", 0.0);
        imported.totalMistakes = root.value("totalMistakes", 0);
        imported.totalNumbersPlaced = root.value("totalNumbersPlaced", 0);
        imported.totalNumbersCleared = root.value("totalNumbersCleared", 0);
        imported.runsWithAutoCandidates = root.value("runsWithAutoCandidates", 0);
        imported.runsWithAutoCheck = root.value("runsWithAutoCheck", 0);
        imported.runsWithConflictHighlight = root.value("runsWithConflictHighlight", 0);
        imported.runsWithAssists = root.value("runsWithAssists", 0);
        imported.runsWithoutAssists = root.value("runsWithoutAssists", 0);

        if (root.contains("difficulties") && root["difficulties"].is_array()) {
            const auto& diffs = root["difficulties"];
            for (size_t i = 0; i < std::min(diffs.size(), imported.difficulties.size()); ++i) {
                const auto& d = diffs[i];
                imported.difficulties[i].gamesStarted = d.value("gamesStarted", 0);
                imported.difficulties[i].gamesCompleted = d.value("gamesCompleted", 0);
                imported.difficulties[i].totalTimeSeconds = d.value("totalTimeSeconds", 0.0);
                imported.difficulties[i].completedTimeSeconds = d.value("completedTimeSeconds", imported.difficulties[i].totalTimeSeconds);
                imported.difficulties[i].bestTimeSeconds = d.value("bestTimeSeconds", 0.0);
                imported.difficulties[i].numbersPlaced = d.value("numbersPlaced", 0);
                imported.difficulties[i].numbersCleared = d.value("numbersCleared", 0);
            }
        }

        stats = std::move(imported);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to import statistics json: " << e.what() << std::endl;
        return false;
    }
}

void StatisticsManager::Save() const {
    nlohmann::json root = ExportJson();

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
    const double clampedTime = std::max(0.0, completionTimeSeconds);

    stats.totalGamesCompleted++;
    stats.totalTimeSeconds += clampedTime;
    stats.totalCompletedTimeSeconds += clampedTime;

    int index = DifficultyToIndex(difficulty);
    auto& diffStats = stats.difficulties[index];
    diffStats.gamesCompleted++;
    diffStats.totalTimeSeconds += clampedTime;
    diffStats.completedTimeSeconds += clampedTime;

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

void StatisticsManager::RecordGameAbandoned(double elapsedSeconds) {
    if (elapsedSeconds <= 0.0) {
        return;
    }

    // Play time should include both completed and discarded runs.
    stats.totalTimeSeconds += std::max(0.0, elapsedSeconds);
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

void StatisticsManager::RecordAppActiveTime(double deltaSeconds) {
    if (deltaSeconds <= 0.0) {
        return;
    }
    stats.totalAppTimeSeconds += deltaSeconds;
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
