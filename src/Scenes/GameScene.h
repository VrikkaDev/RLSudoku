//
// Created by VrikkaDev on 31.1.2024.
//

#ifndef RLSUDOKU_GAMESCENE_H
#define RLSUDOKU_GAMESCENE_H


#include "Scene.h"
#include "Storage/Saveable.h"
#include "Storage/LeaderboardManager.h"
#include <vector>
#include <string>
#include <map>

class TileGrid;
class GameSaveable;

class GameScene : public Scene{
public:
    GameScene();
    GameScene(const std::string& initialBoardStr, const std::string& solutionBoardStr, int difficulty, bool practiceMode);
    explicit GameScene(int difficulty);
    explicit GameScene(bool load);

    void Setup() override;
    void OnResize() override;

    std::unique_ptr<SudokuBoard> board;
    std::unique_ptr<SudokuBoard> orgBoard;
    std::unique_ptr<SudokuBoard> solvedBoard;
    std::map<int, std::string> startNotes = {};
    std::map<int, std::vector<int>> startAutoCandidateRemoved = {};

    // :(
    TileGrid* tileGrid;
    bool newGame = true; // if false then load game from json.

    // Difficulty 0-100
    int difficulty = 0;
    
    // Leaderboard tracking
    std::vector<MoveRecord> moveHistory;
    double puzzleStartTime = 0;
    std::time_t puzzleStartRealTime = 0;  // Real-world timestamp when puzzle started
    bool usedAutoCandidates = false;
    bool usedAutoCheck = false;
    bool usedConflictHighlight = false;
    bool isPracticeRun = false;
    bool usePresetBoard = false;
    std::string presetInitialBoard;
    std::string presetSolutionBoard;
    
    void RecordMove(int tileNumber, int value, double timestamp);
    void RecordCandidateChange(int tileNumber, MoveAction action, double timestamp);
    void SubmitToLeaderboard();

private:
    void RecordAbandonedRunStats();
    void RecordGameStartStats();
    void InitializeFromPreset();
    GameSaveable* gs = nullptr;
    bool runDurationAccounted = false;
};

class GameSaveable : public Saveable{
public:
    explicit GameSaveable(GameScene* scene);
    GameScene* scene;

    nlohmann::json GetJson() override;
    void Load(const nlohmann::json& data) override;
};


#endif //RLSUDOKU_GAMESCENE_H
