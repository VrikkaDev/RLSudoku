//
// Created for RLSudoku replay viewing
//

#include "ReplayScene.h"
#include "GameData.h"
#include "LeaderboardScene.h"
#include "Scenes/Drawables/TileGrid.h"
#include "Scenes/Drawables/GenericButton.h"
#include "Scenes/Drawables/TextWidget.h"
#include "Helpers/TimeHelper.h"
#include "raylib.h"
#include "sudokuGen.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

ReplayScene::ReplayScene(const LeaderboardEntry& entryIn) : Scene(), entry(entryIn) {
    moves = entry.moves;
    std::sort(moves.begin(), moves.end(), [](const MoveRecord& a, const MoveRecord& b) {
        return a.timestamp < b.timestamp;
    });
    maxReplayTime = moves.empty() ? entry.completionTime : std::max(entry.completionTime, moves.back().timestamp);
    if (maxReplayTime <= 0.0) {
        maxReplayTime = moves.empty() ? 0.0 : moves.back().timestamp;
    }

    defaultAutoMode = entry.usedAutoCandidates;
    bool anyAutoMetadata = std::any_of(moves.begin(), moves.end(), [](const MoveRecord& move) {
        return move.autoCandidatesEnabled;
    });
    bool anyCandidateActions = std::any_of(moves.begin(), moves.end(), [](const MoveRecord& move) {
        return move.action != MoveAction::Value;
    });
    legacyAutoMode = defaultAutoMode && !anyAutoMetadata && !anyCandidateActions;
}

void ReplayScene::Setup() {
    currentTime = 0.0;
    playbackSpeed = 1.0;
    playing = true;

    board = std::make_unique<SudokuBoard>(entry.initialBoard.c_str());
    originalBoard = std::make_unique<SudokuBoard>(entry.initialBoard.c_str());
    solvedBoard = std::make_unique<SudokuBoard>(entry.solutionBoard.c_str());

    float boardSize = GetScreenHeight() - 80.0f;
    if (boardSize < 360.0f) {
        boardSize = 360.0f;
    }
    float boardX = 20.0f;
    float boardY = GetScreenHeight() / 2.0f - boardSize / 2.0f;

    tileGrid = new TileGrid(board.get(), originalBoard.get(), solvedBoard.get(), {}, {}, Rectangle{boardX, boardY, boardSize, boardSize});
    drawableStack->AddDrawable(tileGrid);

    tileGrid->OnClick = [](MouseEvent*){};
    tileGrid->OnEvent = [](Event*){};
    tileGrid->enabled = false;
    for (auto* child : tileGrid->children) {
        if (child) {
            child->enabled = false;
        }
    }
    tileGrid->ResetToInitialBoard();

    float panelX = boardX + boardSize + 30.0f;
    float panelWidth = std::max(220.0f, GetScreenWidth() - panelX - 30.0f);
    float textY = 40.0f;

    std::string title = "Replay - " + (entry.playerName.empty() ? std::string("Player") : entry.playerName);
    auto* titleWidget = new TextWidget(title, static_cast<int>(panelX), static_cast<int>(textY), 28, WHITE, false);
    drawableStack->AddDrawable(titleWidget);
    textY += 40.0f;

    std::string difficultyText = "Difficulty: " + std::to_string(entry.difficulty);
    auto* diffWidget = new TextWidget(difficultyText, static_cast<int>(panelX), static_cast<int>(textY), 22, LIGHTGRAY, false);
    drawableStack->AddDrawable(diffWidget);
    textY += 30.0f;

    std::string totalTime = "Completion Time: " + TimeHelper::GetTimeFormatted(entry.completionTime, true);
    auto* timeWidget = new TextWidget(totalTime, static_cast<int>(panelX), static_cast<int>(textY), 22, LIGHTGRAY, false);
    drawableStack->AddDrawable(timeWidget);
    textY += 40.0f;

    timeLabel = new TextWidget("Time: 0:00.000", static_cast<int>(panelX), static_cast<int>(textY), 24, WHITE, false);
    drawableStack->AddDrawable(timeLabel);
    textY += 30.0f;

    statusLabel = new TextWidget("Status: Playing", static_cast<int>(panelX), static_cast<int>(textY), 20, WHITE, false);
    drawableStack->AddDrawable(statusLabel);
    textY += 50.0f;

    float buttonW = panelWidth;
    float buttonH = 48.0f;
    float buttonY = textY;

    playPauseButton = new GenericButton("Pause", Rectangle{panelX, buttonY, buttonW, buttonH});
    playPauseButton->fontSize = 26;
    playPauseButton->OnClick = [this](MouseEvent*) {
        playing = !playing;
        UpdateUiTexts();
    };
    drawableStack->AddDrawable(playPauseButton);
    buttonY += buttonH + 10.0f;

    restartButton = new GenericButton("Restart", Rectangle{panelX, buttonY, buttonW, buttonH});
    restartButton->fontSize = 26;
    restartButton->OnClick = [this](MouseEvent*) {
        currentTime = 0.0;
        playing = true;
        UpdateUiTexts();
    };
    drawableStack->AddDrawable(restartButton);
    buttonY += buttonH + 10.0f;

    stepBackButton = new GenericButton("Step Back", Rectangle{panelX, buttonY, buttonW, buttonH});
    stepBackButton->fontSize = 24;
    stepBackButton->OnClick = [this](MouseEvent*) {
        StepBackward();
        UpdateUiTexts();
    };
    drawableStack->AddDrawable(stepBackButton);
    buttonY += buttonH + 10.0f;

    stepForwardButton = new GenericButton("Step Forward", Rectangle{panelX, buttonY, buttonW, buttonH});
    stepForwardButton->fontSize = 24;
    stepForwardButton->OnClick = [this](MouseEvent*) {
        StepForward();
        UpdateUiTexts();
    };
    drawableStack->AddDrawable(stepForwardButton);
    buttonY += buttonH + 10.0f;

    speedButton = new GenericButton("Speed: 1x", Rectangle{panelX, buttonY, buttonW, buttonH});
    speedButton->fontSize = 24;
    speedButton->OnClick = [this](MouseEvent*) {
        if (playbackSpeed < 1.0f) {
            playbackSpeed = 1.0f;
        } else if (playbackSpeed < 2.0f) {
            playbackSpeed = 2.0f;
        } else if (playbackSpeed < 4.0f) {
            playbackSpeed = 4.0f;
        } else {
            playbackSpeed = 0.5f;
        }
        UpdateUiTexts();
    };
    drawableStack->AddDrawable(speedButton);
    buttonY += buttonH + 20.0f;

    auto* instructions = new TextWidget("Space: Play/Pause  |  Left/Right: Step  |  R: Restart", static_cast<int>(panelX), static_cast<int>(buttonY), 18, GRAY, false);
    drawableStack->AddDrawable(instructions);
    buttonY += 40.0f;

    auto* backButton = new GenericButton("Back", Rectangle{GetScreenWidth() - 220.0f, 20.0f, 200.0f, 50.0f});
    backButton->fontSize = 26;
    backButton->OnClick = [](MouseEvent*) {
        GameData::SetScene(std::make_unique<LeaderboardScene>());
    };
    drawableStack->AddDrawable(backButton);

    ApplyCurrentState();
    UpdateUiTexts();
}

void ReplayScene::OnUpdate() {
    Scene::OnUpdate();

    if (!tileGrid) {
        return;
    }

    double delta = GetFrameTime();

    if (IsKeyPressed(KEY_SPACE)) {
        playing = !playing;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        StepForward();
    }
    if (IsKeyPressed(KEY_LEFT)) {
        StepBackward();
    }
    if (IsKeyPressed(KEY_R)) {
        currentTime = 0.0;
        playing = true;
    }

    if (playing) {
        currentTime += delta * playbackSpeed;
        if (currentTime >= maxReplayTime) {
            currentTime = maxReplayTime;
            playing = false;
        }
    }

    currentTime = std::max(0.0, std::min(currentTime, maxReplayTime));

    ApplyCurrentState();
    UpdateUiTexts();
}

void ReplayScene::OnResize() {
    GameData::SetScene(std::make_unique<ReplayScene>(entry));
}

void ReplayScene::ApplyCurrentState() {
    if (!tileGrid) {
        return;
    }

    tileGrid->ResetToInitialBoard();

    int lastTile = -1;
    bool hasOverride = false;
    bool currentAutoMode = defaultAutoMode;

    if (defaultAutoMode) {
        tileGrid->SetReplayAutoModeForAll(true);
        hasOverride = true;
    }

    bool allowPerMoveAuto = !legacyAutoMode;

    for (const auto& move : moves) {
        if (move.timestamp - 0.0001 > currentTime) {
            break;
        }

        if (allowPerMoveAuto) {
            if (!hasOverride || currentAutoMode != move.autoCandidatesEnabled) {
                tileGrid->SetReplayAutoModeForAll(move.autoCandidatesEnabled);
                hasOverride = true;
                currentAutoMode = move.autoCandidatesEnabled;
            }
        }

        switch (move.action) {
            case MoveAction::Value: {
                int value = (move.value >= 1 && move.value <= 9) ? move.value : 0;
                tileGrid->ApplyReplayValue(move.tileNumber, value);
                lastTile = move.tileNumber;
                break;
            }
            case MoveAction::Notes:
            case MoveAction::ManualCandidates: {
                tileGrid->ApplyReplayCandidateState(move);
                lastTile = move.tileNumber;
                break;
            }
            default:
                break;
        }
    }

    if (hasOverride) {
        tileGrid->SetReplayAutoModeForAll(currentAutoMode);
    }

    tileGrid->UpdateAllAutoCandidates();

    if (lastTile >= 0) {
        tileGrid->SelectTile(lastTile);
    } else {
        tileGrid->ClearSelection();
    }
}

void ReplayScene::UpdateUiTexts() {
    if (timeLabel) {
        std::ostringstream oss;
        oss << "Time: " << TimeHelper::GetTimeFormatted(currentTime, true);
        if (maxReplayTime > 0.0) {
            oss << " / " << TimeHelper::GetTimeFormatted(maxReplayTime, true);
        }
        timeLabel->text = oss.str();
    }

    if (statusLabel) {
        std::ostringstream oss;
        oss << "Status: " << (playing ? "Playing" : "Paused") << "  (";
        if (playbackSpeed == 0.5f) {
            oss << "0.5x";
        } else if (playbackSpeed == 1.0f) {
            oss << "1x";
        } else if (playbackSpeed == 2.0f) {
            oss << "2x";
        } else {
            oss << std::fixed << std::setprecision(1) << playbackSpeed << "x";
        }
        oss << ")";
        statusLabel->text = oss.str();
    }

    if (playPauseButton) {
        playPauseButton->text = playing ? "Pause" : "Play";
    }

    if (speedButton) {
        std::ostringstream oss;
        oss << "Speed: " << std::fixed << std::setprecision(playbackSpeed == 0.5f ? 1 : 0) << playbackSpeed << "x";
        speedButton->text = oss.str();
    }
}

void ReplayScene::StepForward() {
    if (moves.empty()) {
        currentTime = maxReplayTime;
        playing = false;
        return;
    }

    double epsilon = 0.0001;
    for (const auto& move : moves) {
        if (move.timestamp > currentTime + epsilon) {
            currentTime = move.timestamp;
            playing = false;
            return;
        }
    }

    currentTime = maxReplayTime;
    playing = false;
}

void ReplayScene::StepBackward() {
    if (moves.empty()) {
        currentTime = 0.0;
        playing = false;
        return;
    }

    double epsilon = 0.0001;
    for (auto it = moves.rbegin(); it != moves.rend(); ++it) {
        if (it->timestamp < currentTime - epsilon) {
            currentTime = std::max(0.0, it->timestamp);
            playing = false;
            return;
        }
    }

    currentTime = 0.0;
    playing = false;
}
