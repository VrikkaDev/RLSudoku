//
// Created for RLSudoku replay viewing
//

#ifndef RLSUDOKU_REPLAYSCENE_H
#define RLSUDOKU_REPLAYSCENE_H

#include "Scene.h"
#include "Storage/LeaderboardManager.h"

class TileGrid;
class GenericButton;
class TextWidget;
class SudokuBoard;

class ReplayScene : public Scene {
public:
    explicit ReplayScene(const LeaderboardEntry& entry);

    void Setup() override;
    void OnUpdate() override;
    void OnResize() override;

private:
    void ApplyCurrentState();
    void UpdateUiTexts();
    void StepForward();
    void StepBackward();

    LeaderboardEntry entry;
    std::vector<MoveRecord> moves;

    std::unique_ptr<SudokuBoard> board;
    std::unique_ptr<SudokuBoard> originalBoard;
    std::unique_ptr<SudokuBoard> solvedBoard;

    TileGrid* tileGrid = nullptr;
    GenericButton* playPauseButton = nullptr;
    GenericButton* speedButton = nullptr;
    GenericButton* restartButton = nullptr;
    GenericButton* stepBackButton = nullptr;
    GenericButton* stepForwardButton = nullptr;
    TextWidget* timeLabel = nullptr;
    TextWidget* statusLabel = nullptr;

    double currentTime = 0.0;
    double maxReplayTime = 0.0;
    double playbackSpeed = 1.0;
    bool playing = true;
    bool defaultAutoMode = false;
    bool legacyAutoMode = false;
};

#endif // RLSUDOKU_REPLAYSCENE_H
