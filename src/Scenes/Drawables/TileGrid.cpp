//
// Created by VrikkaDev on 31.1.2024.
//

#include "TileGrid.h"
#include "TileButton.h"
#include "Event/KeyboardEvent.h"
#include "Helpers/KeyHelper.h"
#include "GameData.h"
#include "Storage/StorageManager.h"
#include "Scenes/GameScene.h"
#include "ClockWidget.h"
#include "Event/GameEvent.h"
#include "Scenes/Scene.h"
#include <algorithm>

namespace {
std::vector<int> ValuesFromMask(uint16_t mask) {
    std::vector<int> values;
    for (int i = 0; i < 9; ++i) {
        if (mask & (1u << i)) {
            values.push_back(i + 1);
        }
    }
    return values;
}
}

TileGrid::TileGrid() : Drawable(){
}

TileGrid::TileGrid(SudokuBoard* brd, SudokuBoard* orgBrd, SudokuBoard* solved, std::map<int, std::string> sn, std::map<int, std::vector<int>> sacr, Rectangle rec) : Drawable() {
    orgBoard = orgBrd;
    board = brd;
    solvedBoard = solved;
    x = rec.x;
    y = rec.y;

    startNotes = sn;
    startAutoCandidateRemoved = sacr;
    int h = rec.height; // Make sure size is multiple of 9 for consistency
    int remainder = h % 9;
    if (remainder != 0) {
        h = h - remainder + 9;
    }

    width = h;
    height = h;
}

int posToIndex(Vector2 pos) {
    return pos.x * 9 + pos.y;
}

void TileGrid::OnStart() {

    OnClick = [this](MouseEvent* event){
        if(isPaused){
            // Send resume event
            auto* ge = new GameEvent(3, "0");
            GameData::currentScene->eventDispatcher->AddEvent(ge);
            return;
        }
        for (auto* dr : children){
            if (!CheckCollisionPointRec(event->MousePosition, dr->GetRectangle())){
                continue;
            }

            // EventType 2 is RELEASED
            if (event->EventType == 2){
                dr->OnClick(event);
            }
        }
    };

    OnEvent = [this](Event* event){
        if (auto* ke = dynamic_cast<KeyboardEvent*>(event)){

            if(ke->EventType != 1){
                return;
            }

            int num = KeyHelper::GetNumberFromKeyNum(ke->Key);

            // Num keys movement
            if(num == -1){
                int row = selectedTile / 9;
                int column = selectedTile % 9;
                Vector2 vc {-1,-1};
                if(ke->Key == KEY_RIGHT) {
                    vc = {(float)row+1 ,(float)column};
                }else if(ke->Key == KEY_LEFT) {
                    vc = {(float)row-1 ,(float)column};
                }else if(ke->Key == KEY_UP) {
                    vc = {(float)row ,(float)column-1};
                }else if(ke->Key == KEY_DOWN) {
                    vc = {(float)row ,(float)column+1};
                }
                if(vc.x != -1){
                    // Make sure it doesn't go out of range
                    float xx = std::max(0.f,(float)std::min(8.f, vc.x));
                    float yy = std::max(0.f,(float)std::min(8.f, vc.y));
                    vc = {xx, yy};
                    SelectTile(posToIndex(vc));
                }

            } else if(selectedTile != -1 && !isPaused){ // Set the num of selected tile on keypress

                if (num == 0) num = -1;

                auto* dr = this->children[this->selectedTile];
                if (auto* drdre = dynamic_cast<TileButton*>(dr)){
                    drdre->SetText(std::to_string(num));
                }
                // To refresh "options_toggle_hlsamenumbers"
                SelectTile(selectedTile);
            }
        }else if (auto* ge = dynamic_cast<GameEvent*>(event)){
            if(ge->EventType == 3){ // EventType 3 == pause event
                isPaused = ge->Data == "1";
            }
        }
    };

    std::string vstr;
    // Create tiles
    int t = 0;
    for (int i = 0; i < 9; i++){ // Column
        for (int j = 0; j < 9; j++){ // Row

            // Get if dividable by 3 for thicker lines
            float xit = !(i%3) * 2;
            float xjt = !(j%3) * 2;

            float ow = (width/9), oh = (height/9);
            auto rec = Rectangle {(float)x + i * ow + xit, (float)y + j * oh + xjt, ow, oh};

            auto val = orgBoard->at(t);
            vstr = std::to_string(static_cast<int>(val->value));

            // If its 0 make it be nothing
            if(vstr == "0") vstr = "";

            auto* tb = new TileButton(t, vstr.c_str(), solvedBoard->at(t)->value, rec);

            // Now do same with actual board
            val = board->at(t);
            vstr = std::to_string(static_cast<int>(val->value));


            tb->fontSize=ow/1.5;
            tb->parent = this;
            children.push_back(tb);

            if(vstr != "0"){
                tb->text = vstr;
                SetTile(tb->tileNumber, stoi(vstr));
            }

            t++;
        }
    }


    // Needs to be separate because TileButton::OnStart()
    // modifies the board so it messes up the numbers
    for (Drawable* ch : children) {
        ch->OnStart();

        if(auto* drdre = dynamic_cast<TileButton*>(ch)){
            auto it = startNotes.find(drdre->tileNumber);
            if(it != startNotes.end()){
                std::string notesstring = it->second;
                for(char &i : notesstring){
                    std::string astr (1, i);
                    // Only add if doesnt already have
                    auto nit = std::find(drdre->notes.begin(), drdre->notes.end(),stoi(astr));
                    if(nit == drdre->notes.end()){
                        drdre->notes.push_back(stoi(astr));
                    }
                }
            }
        }
    }
    
    // Apply auto candidate removed data
    for (Drawable* ch : children) {
        if(auto* drdre = dynamic_cast<TileButton*>(ch)){
            auto it = startAutoCandidateRemoved.find(drdre->tileNumber);
            if(it != startAutoCandidateRemoved.end()){
                drdre->manuallyRemovedCandidates = it->second;
                // Recalculate auto candidates with the loaded removed candidates
                drdre->calculateAutoCandidates();
            }
        }
    }
    
    // Initialize auto candidates for all tiles after everything is set up
    // Use a small delay to ensure all TileButtons are properly initialized
    UpdateAllAutoCandidates();
}

void TileGrid::Draw() {
    DrawRectangle(x, y, width+2, height+2, color);

    // Draw children aswell
    for (Drawable* tg : children){
        tg->Draw();
    }

    // Draw pause screen
    if(isPaused){
        DrawRectangle(x, y, width-1, height-1, CLITERAL(Color){ 150, 150, 150, 200 });
        DrawTextBC("PAUSED", x, y-20, 48, width, height, color);
        DrawTextBC("Click to resume!", x, y + 20, 32, width, height, color);
    }
}

void TileGrid::SelectTile(int tilenumber) {

    selectedTile = tilenumber;

    nlohmann::json j = GameData::storageManager->GetData("options_toggle_showgridlines");
    nlohmann::json j2 = GameData::storageManager->GetData("options_toggle_hlsamenumbers");

    // Determine the row and column of the tile
    int targetRow = tilenumber / 9;
    int targetColumn = tilenumber % 9;

    // check
    if(tilenumber > children.size()){
        std::cout<<"Tried to set tilenumber " << tilenumber << " that doesnt exist" << std::endl;
        return;
    }

    auto* cb = dynamic_cast<TileButton*>(children[tilenumber]);


    for (auto* d : children){
        if (auto* tb = dynamic_cast<TileButton*>(d)){
            tb->inGridLine = false;
            if (tb->tileNumber == tilenumber){
                tb->selected = true;
                continue;
            }
            // If showgridlines is enabled
            if(j.contains("value") && j["value"]){
                int currentRow = tb->tileNumber / 9;
                int currentColumn = tb->tileNumber % 9;

                if(currentRow == targetRow || currentColumn == targetColumn){
                    tb->inGridLine = true;
                }
            }

            // SameNumber highlight
            if(j2.contains("value") && j2["value"]){
                if(tb->text == cb->text && !tb->text.empty() && tb->text != "-1"){
                    tb->inGridLine = true;
                }
            }

            tb->DeSelect();
        }
    }
}

void TileGrid::CheckIfFinished() {
    // If the board is same as solvedboard. its finished
    this->isFinished = board->operator==(*solvedBoard);
    if (isFinished){
        // Event type 1 = win game event and no data needed
        auto* e = new GameEvent(1, "");
        GameData::currentScene->eventDispatcher->AddEvent(e);
        
        // Submit to leaderboard
        if(auto* gs = dynamic_cast<GameScene*>(GameData::currentScene.get())){
            gs->SubmitToLeaderboard();
        }
    }
}

void TileGrid::RecordTileChange(int tileNumber, int value) {
    if(auto* gs = dynamic_cast<GameScene*>(GameData::currentScene.get())){
        // Get current time from clock widget
        double currentTime = 0;
        for(const auto& dr : gs->drawableStack->drawables){
            if(auto* cw = dynamic_cast<ClockWidget*>(dr)){
                currentTime = cw->GetCurrentTime();
                break;
            }
        }
        gs->RecordMove(tileNumber, value, currentTime);
    }
}

void TileGrid::SetTile(int tilenumber, int value) {

    if (value < 0) value = 0;

    tileValues[tilenumber] = value;
    board->setValue(tilenumber, value);
    
    // Only update auto candidates if all tiles are initialized (check if we have 81 children)
    if (children.size() == 81) {
        UpdateAllAutoCandidates();
    }
}

void TileGrid::UpdateAllAutoCandidates() {
    // Update auto candidates for all tiles
    for (auto* child : children) {
        if (child && dynamic_cast<TileButton*>(child)) {
            if (auto* tb = dynamic_cast<TileButton*>(child)) {
                tb->calculateAutoCandidates();
            }
        }
    }
}

void TileGrid::RemoveCandidatesFromRelatedTiles(int tileNumber, int value) {
    // Check if auto-remove candidates option is enabled
    nlohmann::json autoRemoveEnabled = GameData::storageManager->GetData("options_toggle_autoremovecandidates");
    if (!autoRemoveEnabled.contains("value") || !autoRemoveEnabled["value"]) {
        return; // Feature disabled
    }
    
    int targetRow = tileNumber / 9;
    int targetCol = tileNumber % 9;
    int targetBox = (targetRow / 3) * 3 + (targetCol / 3);
    
    // Remove the candidate from all related tiles
    for (auto* child : children) {
        if (auto* tb = dynamic_cast<TileButton*>(child)) {
            if (tb->tileNumber == tileNumber) continue; // Skip the tile we just filled
            
            int otherRow = tb->tileNumber / 9;
            int otherCol = tb->tileNumber % 9;
            int otherBox = (otherRow / 3) * 3 + (otherCol / 3);
            
            // Check if in same row, column, or box
            bool isRelated = (otherRow == targetRow) || 
                           (otherCol == targetCol) || 
                           (otherBox == targetBox);
            
            if (isRelated) {
                // Remove the value from manual notes if it exists
                auto it = std::find(tb->notes.begin(), tb->notes.end(), value);
                if (it != tb->notes.end()) {
                    tb->notes.erase(it);
                }
            }
        }
    }
}

void TileGrid::ClearSelection() {
    selectedTile = -1;
    for (auto* child : children) {
        if (auto* tb = dynamic_cast<TileButton*>(child)) {
            tb->selected = false;
            tb->inGridLine = false;
        }
    }
}

void TileGrid::ResetToInitialBoard() {
    if (!board || !orgBoard) {
        return;
    }

    ClearSelection();

    for (auto* child : children) {
        if (auto* tb = dynamic_cast<TileButton*>(child)) {
            int original = static_cast<int>(orgBoard->at(tb->tileNumber)->value);
            if (original > 0) {
                tb->text = std::to_string(original);
            } else {
                tb->text.clear();
            }

            tb->showIsWrong = false;
            tb->showConflicts = false;
            tb->conflicts.clear();
            tb->notes.clear();
            tb->autoCandidates.clear();
            tb->manuallyRemovedCandidates.clear();
            tb->SetReplayMode(false, false);

            int stored = original > 0 ? original : 0;
            tileValues[tb->tileNumber] = stored;
            board->setValue(tb->tileNumber, stored);
        }
    }

    UpdateAllAutoCandidates();
}

void TileGrid::ApplyReplayValue(int tileNumber, int value) {
    if (!board) {
        return;
    }

    if (tileNumber < 0 || tileNumber >= static_cast<int>(children.size())) {
        return;
    }

    value = std::clamp(value, 0, 9);

    if (auto* tb = dynamic_cast<TileButton*>(children[tileNumber])) {
        if (tb->permanent) {
            return;
        }

        tb->text = value > 0 ? std::to_string(value) : "";
        tb->showIsWrong = false;
        tb->showConflicts = false;
        tb->conflicts.clear();
        tb->notes.clear();
        tb->manuallyRemovedCandidates.clear();
    }

    tileValues[tileNumber] = value;
    board->setValue(tileNumber, value);
}

void TileGrid::RecordCandidateChange(int tileNumber, MoveAction action) {
    if (auto* gs = dynamic_cast<GameScene*>(GameData::currentScene.get())) {
        double currentTime = 0.0;
        for (const auto& dr : gs->drawableStack->drawables) {
            if (auto* cw = dynamic_cast<ClockWidget*>(dr)) {
                currentTime = cw->GetCurrentTime();
                break;
            }
        }
        gs->RecordCandidateChange(tileNumber, action, currentTime);
    }
}

void TileGrid::ApplyReplayCandidateState(const MoveRecord& move) {
    int tileNumber = move.tileNumber;
    if (tileNumber < 0 || tileNumber >= static_cast<int>(children.size())) {
        return;
    }

    if (auto* tb = dynamic_cast<TileButton*>(children[tileNumber])) {
        tb->SetReplayMode(true, move.autoCandidatesEnabled);

        if (move.autoCandidatesEnabled) {
            tb->notes.clear();
            tb->manuallyRemovedCandidates = ValuesFromMask(move.removedMask);
        } else {
            tb->manuallyRemovedCandidates.clear();
            tb->notes = ValuesFromMask(move.notesMask);
        }
    }
}

void TileGrid::SetReplayAutoModeForAll(bool autoMode) {
    for (auto* child : children) {
        if (auto* tb = dynamic_cast<TileButton*>(child)) {
            tb->SetReplayMode(true, autoMode);
        }
    }
}

