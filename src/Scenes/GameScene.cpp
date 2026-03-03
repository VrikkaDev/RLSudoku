//
// Created by VrikkaDev on 31.1.2024.
//

#include "GameScene.h"
#include "Scenes/Drawables/GenericButton.h"
#include "GameData.h"
#include "MainMenuScene.h"
#include "Scenes/Drawables/TileButton.h"
#include "Scenes/Drawables/TileGrid.h"
#include "Scenes/Drawables/ClockWidget.h"
#include "Scenes/Drawables/NumberButtons.h"
#include "Scenes/Drawables/ConfigToggleButton.h"
#include "Scenes/Drawables/TextWidget.h"
#include "Storage/StorageManager.h"
#include "Storage/StatisticsManager.h"
#include "Storage/RemoteSyncManager.h"

#include "JCZSolve.h"
#include "Scenes/Drawables/WinScreen.h"
#include <algorithm>

namespace {
uint16_t MaskFromVector(const std::vector<int>& values) {
    uint16_t mask = 0;
    for (int value : values) {
        if (value >= 1 && value <= 9) {
            mask |= static_cast<uint16_t>(1u << (value - 1));
        }
    }
    return mask;
}

TileButton* GetTileButtonAt(TileGrid* grid, int tileNumber) {
    if (!grid || tileNumber < 0) {
        return nullptr;
    }
    if (tileNumber >= static_cast<int>(grid->children.size())) {
        return nullptr;
    }
    return dynamic_cast<TileButton*>(grid->children[tileNumber]);
}

bool IsAutoCandidatesEnabled() {
    nlohmann::json autoCandidatesEnabled = GameData::storageManager->GetData("options_toggle_autocandidates");
    return autoCandidatesEnabled.contains("value") && autoCandidatesEnabled["value"];
}
}

GameScene::GameScene() : Scene(){
}

GameScene::GameScene(const std::string& initialBoardStr, const std::string& solutionBoardStr, int difficultyValue, bool practiceMode)
    : Scene() {
    difficulty = difficultyValue;
    isPracticeRun = practiceMode;
    usePresetBoard = true;
    presetInitialBoard = initialBoardStr;
    presetSolutionBoard = solutionBoardStr;

    InitializeFromPreset();
    RecordGameStartStats();
}

std::string getRandomLine(const std::string& filename) {
    std::string line;
    std::string selected;
    std::ifstream file(filename);

    if (file.is_open()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0, 1);

        int n = 0;
        while (std::getline(file, line)) {
            ++n;
            if (dis(gen) < 1.0 / n) {
                selected = line;
            }
        }
        file.close();
    } else {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }

    return std::move(selected);
}

GameScene::GameScene(int difficulty) : Scene(){
    this->difficulty = difficulty;

    std::string final_solve;

    // Define the number of threads you want to use
    const int num_threads = 4;

    // Create a vector to hold the threads
    std::vector<std::thread> threads;


    int loop_amount = 0;
    while (true) {
        if(difficulty > 45){
            if(difficulty == 50){
                std::string st = getRandomLine("./assets/puzzles/hard.txt");
                board = std::make_unique<SudokuBoard>(st.c_str());
            }else if(difficulty == 60){
                std::string st = getRandomLine("./assets/puzzles/hardest.txt");
                board = std::make_unique<SudokuBoard>(st.c_str());
            }
        }else{
            board = std::make_unique<SudokuBoard>(genBoard(difficulty));
        }
        std::string st = board->parser();
        char solvstr[81];
        int corcount = JCZSolver(st.c_str(), solvstr, 2);
        if (corcount == 1) {
            final_solve = std::string(solvstr);
            break;
        }else if (corcount <= 0){
            std::cout<<"Couldnt find any solutions for puzzle"<<std::endl;
        }else {
            std::cout<<"Found too many solutions for puzzle. Found: "<<corcount<<" solutions"<<std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

           

        loop_amount++;
        if(loop_amount >= 20){
            std::cout<<"Couldnt solve the sudoku or someting in 20 tries"<<std::endl;
            if (GameData::remoteSyncManager) {
                GameData::remoteSyncManager->ForceSync();
            }
            GameData::SetScene(std::make_unique<MainMenuScene>());
            return;
        }
    }


    orgBoard = std::make_unique<SudokuBoard>(board->parser().c_str());
    solvedBoard = std::make_unique<SudokuBoard>(final_solve.c_str());

    RecordGameStartStats();
}

GameScene::GameScene(bool load) {
    if(load){
        newGame = false;
    }
}

void GameScene::Setup() {

    // Capture the real-world start time for leaderboard
    puzzleStartRealTime = std::time(nullptr);

    // Game Clock
    float cw = (GetScreenWidth() - (20 + GetScreenHeight() - 80 + 20))/2,
    ch = GetScreenHeight()/7, cx = 20 + GetScreenHeight() - 80 + 20, cy = GetScreenHeight() - ch - 20;
    auto cb = new ClockWidget(Rectangle{cx,cy,cw,ch});
    drawableStack->AddDrawable(cb);

    if (!isPracticeRun) {
        gs = new GameSaveable(this);
        // i just dont care :)
        nlohmann::json json = GameData::storageManager->GetData("game_save");
        gs->Load(json);
    } else {
        gs = nullptr;
    }

    // Sudoku Tilegrid
    float tw = GetScreenHeight()-80, th = GetScreenHeight()-80, tx = 20, ty = GetScreenHeight()/2 - th/2;
    auto tb = new TileGrid(board.get(), orgBoard.get(), solvedBoard.get(), startNotes, startAutoCandidateRemoved, Rectangle{tx,ty,tw,th});
    tileGrid = tb;
    drawableStack->AddDrawable(tb);

    // Number Buttons
    int wwh = GetScreenWidth() - GetScreenHeight();
    wwh = std::min((float)wwh, cy-ch);
    float nw = wwh, nh = wwh, nx = GetScreenWidth() - nw - 20, ny = 20;
    auto nb = new NumberButtons(Rectangle{nx,ny,nw,nh});
    drawableStack->AddDrawable(nb);

    // Win Screen
    float ww = GetScreenWidth(), wh = GetScreenHeight(), wx = 0, wy = 0;
    auto wb = new WinScreen(Rectangle{wx,wy,ww,wh});
    drawableStack->AddDrawable(wb);

    if (isPracticeRun) {
        float messageX = cx + cw + 10;
        float messageY = ny + nh + 20;
        auto* practiceNotice = new TextWidget("Practice run — no leaderboard submission.", static_cast<int>(messageX), static_cast<int>(messageY), 18, LIGHTGRAY, false);
        drawableStack->AddDrawable(practiceNotice);
    }

    // Auto Candidates toggle button
    float acw = cw - 20, ach = ch/2 - 5, acx = cx + cw + 10, acy = cy - ach - 10;
    auto acb = new ConfigToggleButton("options_toggle_autocandidates", "Auto Candidates", Rectangle{acx, acy, acw, ach});
    acb->fontSize = ach/2.5;
    
    // Store the original OnClick behavior and extend it
    //acb->OnStart(); // This sets up the default toggle behavior
    acb->OnClick = [this, acb](MouseEvent* event) {
        // Toggle value and save toggle setting
        acb->value = !acb->value;
        GameData::storageManager->Save();
        
        // Refresh scene
        OnResize();
    };
    
    drawableStack->AddDrawable(acb);

    // Back button
    float bw = cw - 20,
    bh = ch, bx = cx + cw + 10, by = cy;
    auto bb = new GenericButton("Back", Rectangle{bx,by,bw,bh});
    bb->fontSize = bh/2;
    bb->OnClick = [](MouseEvent* event) {
        if (GameData::remoteSyncManager) {
            GameData::remoteSyncManager->ForceSync();
        }
        GameData::SetScene(std::make_unique<MainMenuScene>());
    };
    drawableStack->AddDrawable(bb);
}

void GameScene::OnResize() {
    if (usePresetBoard) {
        GameData::SetScene(std::make_unique<GameScene>(presetInitialBoard, presetSolutionBoard, difficulty, isPracticeRun));
        return;
    }
    GameData::SetScene(std::make_unique<GameScene>(true));
}


// -------------------GAMESAVEABLE---------------------

GameSaveable::GameSaveable(GameScene* scene) : Saveable("game_save"){
    this->scene = scene;
}

nlohmann::json GameSaveable::GetJson() {

    if(scene->tileGrid->isFinished){
        return "";
    }

    nlohmann::json json;
    // Save board and solution
    json["tileData"] = scene->board->parser();
    json["orgData"] = scene->orgBoard->parser();
    json["solData"] = scene->solvedBoard->parser();
    json["difficulty"] = scene->difficulty;
    double time = 0.0;
    // Get time from timewidget
    for(const auto& dr : scene->drawableStack->drawables){
        if(auto* cw = dynamic_cast<ClockWidget*>(dr)){
            time = cw->GetCurrentTime();
            break;
        }
    }
    json["time"] = time;

    // Get notes
    std::string notes = {};
    for (const auto& d : scene->tileGrid->children) {
        if (auto* tb = dynamic_cast<TileButton*>(d)) {
            std::string notesstring;
            for(int i : tb->notes){
                if (notesstring.find(std::to_string(i)) == std::string::npos){
                    notesstring += std::to_string(i);
                }
            }
            if(notesstring.empty()){
                continue;
            }
            notes += "|" + std::to_string(tb->tileNumber) + "," + notesstring;
        }
    }
    json["notes"] = notes;

    // Get auto candidate removed notes
    std::string autoCandidateRemoved = {};
    for (const auto& d : scene->tileGrid->children) {
        if (auto* tb = dynamic_cast<TileButton*>(d)) {
            if (tb->manuallyRemovedCandidates.empty()) {
                continue;
            }
            std::string removedString;
            for(int removed : tb->manuallyRemovedCandidates){
                removedString += std::to_string(removed);
            }
            autoCandidateRemoved += "|" + std::to_string(tb->tileNumber) + "," + removedString;
        }
    }
    json["autoCandidateRemoved"] = autoCandidateRemoved;

    return json;
}

void GameSaveable::Load(const nlohmann::json& data) {
    if(scene->newGame || data.empty()){
        return;
    }
    scene->newGame = true;

    std::string td = (std::string)data["tileData"];
    std::string od = (std::string)data["orgData"];
    std::string sd = (std::string)data["solData"];

    scene->orgBoard = std::make_unique<SudokuBoard>(od.c_str());
    scene->board = std::make_unique<SudokuBoard>(td.c_str());
    scene->solvedBoard = std::make_unique<SudokuBoard>(sd.c_str());
    scene->difficulty = data["difficulty"];

    std::string ss = data["notes"];

    std::vector<std::string> nots = StringHelper::SplitString(ss.c_str(), '|');

    for(const auto& tx : nots){
        if(tx.empty()){
            continue;
        }

        std::vector<std::string> ton = StringHelper::SplitString(tx.c_str(), ',');
        int ind = stoi(ton[0]);

        scene->startNotes[ind] = ton[1];
    }

    // Load auto candidate removed notes
    if (data.contains("autoCandidateRemoved")) {
        std::string acr = data["autoCandidateRemoved"];
        std::vector<std::string> removedNotes = StringHelper::SplitString(acr.c_str(), '|');

        for(const auto& tx : removedNotes){
            if(tx.empty()){
                continue;
            }

            std::vector<std::string> ton = StringHelper::SplitString(tx.c_str(), ',');
            int tileInd = stoi(ton[0]);

            // Parse the removed candidates as individual digits from concatenated string
            std::string removedStr = ton[1];
            std::vector<int> removedCandidates;
            for(char &c : removedStr){
                std::string digitStr(1, c);
                removedCandidates.push_back(stoi(digitStr));
            }

            scene->startAutoCandidateRemoved[tileInd] = removedCandidates;
        }
    }

    // Set time for timewidget
    for(const auto& dr : scene->drawableStack->drawables){
        if(auto* cw = dynamic_cast<ClockWidget*>(dr)){
            cw->SetTime((double)data["time"]);
            // Pause
            cw->OnClick(nullptr);
        }
    }
}

// Leaderboard tracking methods
void GameScene::RecordMove(int tileNumber, int value, double timestamp) {
    TileButton* tileButton = GetTileButtonAt(tileGrid, tileNumber);
    uint16_t notesMask = tileButton ? MaskFromVector(tileButton->notes) : 0;
    uint16_t removedMask = tileButton ? MaskFromVector(tileButton->manuallyRemovedCandidates) : 0;
    bool autoMode = IsAutoCandidatesEnabled();

    moveHistory.emplace_back(
        tileNumber,
        value,
        timestamp,
        notesMask,
        removedMask,
        autoMode,
        MoveAction::Value
    );
}

void GameScene::SubmitToLeaderboard() {
    if (isPracticeRun) {
        return;
    }
    
    // Get final time from clock widget
    double finalTime = 0;
    for(const auto& dr : drawableStack->drawables){
        if(auto* cw = dynamic_cast<ClockWidget*>(dr)){
            finalTime = cw->GetCurrentTime();
            break;
        }
    }
    
    // Check settings used during run
    nlohmann::json autoCandidatesEnabled = GameData::storageManager->GetData("options_toggle_autocandidates");
    nlohmann::json autoCheckEnabled = GameData::storageManager->GetData("options_toggle_autocheck");
    nlohmann::json conflictHighlightEnabled = GameData::storageManager->GetData("options_toggle_hlconflicts");
    
    usedAutoCandidates = autoCandidatesEnabled.contains("value") && autoCandidatesEnabled["value"];
    usedAutoCheck = autoCheckEnabled.contains("value") && autoCheckEnabled["value"];
    usedConflictHighlight = conflictHighlightEnabled.contains("value") && conflictHighlightEnabled["value"];
    
    // Create leaderboard entry
    LeaderboardEntry entry;
    entry.playerName = "Vrikka"; // TODO: Add name input dialog
    entry.completionTime = finalTime;
    entry.initialBoard = orgBoard->parser();
    entry.solutionBoard = solvedBoard->parser();
    entry.difficulty = difficulty;
    entry.usedAutoCandidates = usedAutoCandidates;
    entry.usedAutoCheck = usedAutoCheck;
    entry.usedConflictHighlight = usedConflictHighlight;
    entry.moves = moveHistory;
    entry.startedAt = puzzleStartRealTime;
    entry.completedAt = std::time(nullptr);
    
    GameData::leaderboardManager->AddEntry(entry);

    if (GameData::remoteSyncManager) {
        GameData::remoteSyncManager->QueueLeaderboardSubmission(entry);
    }

    if (GameData::statisticsManager) {
        GameData::statisticsManager->RecordGameCompleted(
            difficulty,
            finalTime,
            usedAutoCandidates,
            usedAutoCheck,
            usedConflictHighlight
        );

        if (GameData::remoteSyncManager) {
            GameData::remoteSyncManager->QueueStatisticsUpdate();
        }
    }
}

void GameScene::RecordGameStartStats() {
    if (isPracticeRun) {
        return;
    }
    if (GameData::statisticsManager) {
        GameData::statisticsManager->RecordGameStart(difficulty);
    }
}

void GameScene::InitializeFromPreset() {
    if (!usePresetBoard) {
        return;
    }

    if (presetInitialBoard.size() == 81 && presetSolutionBoard.size() == 81) {
        board = std::make_unique<SudokuBoard>(presetInitialBoard.c_str());
        orgBoard = std::make_unique<SudokuBoard>(presetInitialBoard.c_str());
        solvedBoard = std::make_unique<SudokuBoard>(presetSolutionBoard.c_str());
    }
}

void GameScene::RecordCandidateChange(int tileNumber, MoveAction action, double timestamp) {
    TileButton* tileButton = GetTileButtonAt(tileGrid, tileNumber);
    if (!tileButton) {
        return;
    }

    uint16_t notesMask = MaskFromVector(tileButton->notes);
    uint16_t removedMask = MaskFromVector(tileButton->manuallyRemovedCandidates);
    bool autoMode = IsAutoCandidatesEnabled();
    int currentValue = 0;
    if (!tileButton->text.empty() && tileButton->text != "-1") {
        try {
            currentValue = std::stoi(tileButton->text);
        } catch (...) {
            currentValue = 0;
        }
    }

    moveHistory.emplace_back(
        tileNumber,
        currentValue,
        timestamp,
        notesMask,
        removedMask,
        autoMode,
        action
    );
}
