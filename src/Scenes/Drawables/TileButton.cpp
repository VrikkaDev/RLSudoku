//
// Created by VrikkaDev on 31.1.2024.
//

#include "TileButton.h"
#include "TileGrid.h"
#include "GameData.h"
#include "Storage/StorageManager.h"
#include "Storage/StatisticsManager.h"
#include "Scenes/GameScene.h"
#include "Storage/LeaderboardManager.h"
#include <set>
#include <algorithm>

namespace {
bool IsDarkModeEnabled() {
    if (!GameData::storageManager) {
        return false;
    }
    nlohmann::json mode = GameData::storageManager->GetData("options_toggle_darkmode");
    return mode.contains("value") && mode["value"].is_boolean() && mode["value"];
}
}

TileButton::TileButton() : Drawable() {

}

TileButton::TileButton(int tilenum, const char* txt, int correctNum, Rectangle rec) : Drawable() {

    // If input text is something set tile as permanent
    if(strcmp(txt, "") != 0){
        permanent = true;
    }

    tileNumber = tilenum;
    this->correctNum = correctNum;
    text = txt;

    x = rec.x;
    y = rec.y;
    width = rec.width;
    height = rec.height;
}

void TileButton::OnStart() {
    OnClick = [this](MouseEvent* event){

        if (auto* p = dynamic_cast<TileGrid*>(parent)){
            p->SelectTile(tileNumber);
        }
    };

    try{
        if(auto* tg = dynamic_cast<TileGrid*>(parent)){
            auto s = std::string(text);
            tg->SetTile(tileNumber, stoi(s));
        }
    }
    catch(std::exception& e){
    }

}

void TileButton::Draw() {
    const bool darkMode = IsDarkModeEnabled();
    if (darkMode) {
        if (color.r == LIGHTGRAY.r && color.g == LIGHTGRAY.g && color.b == LIGHTGRAY.b && color.a == LIGHTGRAY.a) {
            color = CLITERAL(Color){45, 45, 52, 255};
        }
        if (selectedColor.r == DARKGRAY.r && selectedColor.g == DARKGRAY.g && selectedColor.b == DARKGRAY.b && selectedColor.a == DARKGRAY.a) {
            selectedColor = CLITERAL(Color){92, 92, 104, 255};
        }
        if (gridlineColor.r == GRAY.r && gridlineColor.g == GRAY.g && gridlineColor.b == GRAY.b && gridlineColor.a == GRAY.a) {
            gridlineColor = CLITERAL(Color){70, 70, 80, 255};
        }
        if (textColor2.r == DARKGRAY.r && textColor2.g == DARKGRAY.g && textColor2.b == DARKGRAY.b && textColor2.a == DARKGRAY.a) {
            textColor2 = CLITERAL(Color){205, 205, 214, 255};
        }
        if (textColor3.r == BLACK.r && textColor3.g == BLACK.g && textColor3.b == BLACK.b && textColor3.a == BLACK.a) {
            textColor3 = CLITERAL(Color){238, 238, 244, 255};
        }
    } else {
        if (color.r == 45 && color.g == 45 && color.b == 52 && color.a == 255) {
            color = LIGHTGRAY;
        }
        if (selectedColor.r == 92 && selectedColor.g == 92 && selectedColor.b == 104 && selectedColor.a == 255) {
            selectedColor = DARKGRAY;
        }
        if (gridlineColor.r == 70 && gridlineColor.g == 70 && gridlineColor.b == 80 && gridlineColor.a == 255) {
            gridlineColor = GRAY;
        }
        if (textColor2.r == 205 && textColor2.g == 205 && textColor2.b == 214 && textColor2.a == 255) {
            textColor2 = DARKGRAY;
        }
        if (textColor3.r == 238 && textColor3.g == 238 && textColor3.b == 244 && textColor3.a == 255) {
            textColor3 = BLACK;
        }
    }

    bool isHovering = CheckCollisionPointRec(GetMousePosition(), GetRectangle());
    bool isPressed = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

    // Draw outline
    DrawRectangle(x, y, width, height, selectedColor);
    Color ccol = selected ? selectedColor : inGridLine ? gridlineColor : color;
    DrawRectangle(x+1, y+1, width-2, height-2, ccol);

    if (selected && isHovering && (text.empty() || text == "-1") ){
        int t = 1;
        // Draw small numbers inside
        for (int i = 0; i < 3; i++){
            for (int j = 0; j < 3; j++){
                std::string st = std::to_string(t);
                auto trec = Rectangle {(float)x + j * width/3, (float)y + i * height/3, (float)fontSize/2, (float)fontSize/2};
                bool isHoveringTrec = CheckCollisionPointRec(GetMousePosition(), trec);

                // Check if lmb pressed and toggle note if so
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && isHoveringTrec){
                    nlohmann::json autoCandidatesEnabled = GameData::storageManager->GetData("options_toggle_autocandidates");
                    bool showAutoCandidates = autoCandidatesEnabled.contains("value") && autoCandidatesEnabled["value"];
                    if (replayOverrideMode) {
                        showAutoCandidates = replayAutoMode;
                    }

                    if (showAutoCandidates) {
                        // Auto candidates mode - toggle manually removed candidates
                        auto it = std::find(manuallyRemovedCandidates.begin(), manuallyRemovedCandidates.end(), t);
                        if (it != manuallyRemovedCandidates.end()) {
                            // Was removed, bring it back
                            manuallyRemovedCandidates.erase(it);
                        } else {
                            // Add to removed list
                            manuallyRemovedCandidates.push_back(t);
                        }
                        // Recalculate auto candidates
                        calculateAutoCandidates();
                        
                    } else {
                        // Manual notes mode
                        auto it = std::find(notes.begin(), notes.end(), t);
                        if (it != notes.end()){
                            notes.erase(it);
                        } else{
                            notes.push_back(t);
                        }
                    }

                    if (auto* tg = dynamic_cast<TileGrid*>(parent)) {
                        MoveAction action = showAutoCandidates ? MoveAction::ManualCandidates : MoveAction::Notes;
                        tg->RecordCandidateChange(tileNumber, action);
                    }
                }

                // Makes the notes invisible if far enough when setting notes
                // Calculate the Euclidean distance between the points
                float distance = Vector2Distance(Vector2 {trec.x, trec.y}, GetMousePosition());
                float alpha = 1.f - (distance / (fontSize/ 1.5));
                alpha = Clamp(alpha, 0.0f, 1.0f);
                alpha *= 255;

                auto col = textColor;

                nlohmann::json autoCandidatesEnabled = GameData::storageManager->GetData("options_toggle_autocandidates");
                bool showAutoCandidates = autoCandidatesEnabled.contains("value") && autoCandidatesEnabled["value"];
                if (replayOverrideMode) {
                    showAutoCandidates = replayAutoMode;
                }
                
                bool alr = false;
                if (showAutoCandidates) {
                    // Auto candidates mode - show if it's an auto candidate and not manually removed
                    bool isAutoCandidate = std::find(autoCandidates.begin(), autoCandidates.end(), t) != autoCandidates.end();
                    bool isManuallyRemoved = std::find(manuallyRemovedCandidates.begin(), manuallyRemovedCandidates.end(), t) != manuallyRemovedCandidates.end();
                    alr = isAutoCandidate && !isManuallyRemoved;
                    if (alr) {
                        // Make auto candidates slightly more transparent
                        col.a = (unsigned char)(col.a * 0.7f);
                    }
                } else {
                    // Manual notes mode
                    alr = std::find(notes.begin(), notes.end(), t) != notes.end();
                }

                // Draw with 255 alpha if in notes already
                col.a = alr ? 255.f : alpha;

                DrawTextBC(st.c_str(), trec.x, trec.y,
                           fontSize/2, fontSize/2, fontSize/2, col);
                t++;
            }
        }
    }
    // Draw big number if it exists
    if (!text.empty() && text != "-1"){
        Color color1 = showIsWrong ? wrongNumColor : selected ? textColor : permanent ? textColor3 : textColor2;
        DrawTextBC(text.c_str(), x, y, fontSize*1.25, width, height, color1);
    }else { // Else draw the notes or auto candidates
        nlohmann::json autoCandidatesEnabled = GameData::storageManager->GetData("options_toggle_autocandidates");
        bool showAutoCandidates = autoCandidatesEnabled.contains("value") && autoCandidatesEnabled["value"];
        if (replayOverrideMode) {
            showAutoCandidates = replayAutoMode;
        }
        
        int t = 1;
        // Draw small numbers inside
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                bool shouldShow = false;
                Color candidateColor = selected ? textColor : textColor2;
                
                // Check if this number should be shown
                if (showAutoCandidates) {
                    // Auto candidates mode - show auto candidates that aren't manually removed
                    bool isAutoCandidate = std::find(autoCandidates.begin(), autoCandidates.end(), t) != autoCandidates.end();
                    bool isManuallyRemoved = std::find(manuallyRemovedCandidates.begin(), manuallyRemovedCandidates.end(), t) != manuallyRemovedCandidates.end();
                    shouldShow = isAutoCandidate && !isManuallyRemoved;
                    // Make auto candidates slightly more transparent
                    candidateColor.a = (unsigned char)(candidateColor.a * 0.7f);
                } else {
                    // Manual notes mode - show only manual notes
                    shouldShow = std::find(notes.begin(), notes.end(), t) != notes.end();
                }
                
                if (!shouldShow) {
                    t++;
                    continue;
                }
                
                auto trec = Rectangle{(float) x + j * width / 3, (float) y + i * height / 3, (float) fontSize / 2,
                                      (float) fontSize / 2};
                std::string st = std::to_string(t);
                
                DrawTextBC(st.c_str(), trec.x, trec.y,
                           fontSize / 2, fontSize / 2, fontSize / 2, candidateColor);
                t++;
            }
        }
    }

    // Draw red ball if conflict
    if(showConflicts){
        float radius = 5;
        DrawCircle(x+width-radius,y+height-radius,radius, wrongNumColor);
    }
}

void TileButton::DeSelect() {
    selected = false;
}

void TileButton::SetText(const std::string& str) {
    if (permanent) {
        return;
    }

    if (str == text) {
        return;
    }

    bool hadValueBefore = (!text.empty() && text != "-1");

    nlohmann::json j = GameData::storageManager->GetData("options_toggle_autocheck");

    int numericValue = 0;
    try {
        numericValue = std::stoi(str);
    } catch (...) {
        numericValue = 0;
    }

    bool isPlacement = numericValue > 0;

    auto* gsScene = dynamic_cast<GameScene*>(GameData::currentScene.get());
    bool allowStats = gsScene && !gsScene->isPracticeRun;
    int difficulty = gsScene ? gsScene->difficulty : 0;

    if (allowStats && GameData::statisticsManager) {
        if (isPlacement) {
            GameData::statisticsManager->RecordNumberPlaced(difficulty);
        } else {
            GameData::statisticsManager->RecordNumberCleared(difficulty, hadValueBefore);
        }
    }

    if(auto* tg = dynamic_cast<TileGrid*>(parent)){
        tg->SetTile(tileNumber, numericValue);
        tg->CheckIfFinished();
        
        // Record move for leaderboard
        tg->RecordTileChange(tileNumber, std::max(0, numericValue));
    }

    if (allowStats && isPlacement && numericValue != correctNum) {
        if (GameData::statisticsManager) {
            GameData::statisticsManager->RecordMistake();
        }
    }


    // AutoCheck
    if (isPlacement && j.contains("value") && j["value"]) {
        int si = numericValue;
        if (si != correctNum) {
            showIsWrong = true;
            text = str;
            return;
        }

    }

    showIsWrong = false;
    text = str;
    
    // Clear manually removed candidates when placing a number
    if (isPlacement) {
        manuallyRemovedCandidates.clear();
        notes.clear(); // Also clear manual notes when placing a number
        
        // Remove this number from manual candidates in related tiles
        if(auto* tg = dynamic_cast<TileGrid*>(parent)){
            tg->RemoveCandidatesFromRelatedTiles(tileNumber, numericValue);
        }
    }
    
    CheckForConflicts();
    
    // Update auto candidates for all tiles when this tile's value changes
    if(auto* tg = dynamic_cast<TileGrid*>(parent)){
        tg->UpdateAllAutoCandidates();
    }
}

void TileButton::CheckForConflicts() {

    nlohmann::json j2 = GameData::storageManager->GetData("options_toggle_hlconflicts");

    if (j2.contains("value") && j2["value"]) {

        int targetRow = tileNumber / 9;
        int targetColumn = tileNumber % 9;

        auto childs = parent->children;

        // Function to check conflicts and add to the vector
        auto checkAndAddConflict = [&](int index) {
            if (auto* tb = dynamic_cast<TileButton*>(childs[index])) {
                if(tb->tileNumber == tileNumber){
                    return;
                }
                if (tb->text == text && !text.empty() && text != "-1") {
                    addConflict(tb->tileNumber);
                    tb->addConflict(tileNumber);
                }else{
                    tb->removeConflict(tileNumber);
                    removeConflict(tb->tileNumber);
                }
            }
        };

        // Check conflicts in the same row
        for (int i = targetRow * 9; i < (targetRow + 1) * 9; ++i) {
            checkAndAddConflict(i);
        }

        // Check conflicts in the same column
        for (int i = targetColumn; i < 81; i += 9) {
            checkAndAddConflict(i);
        }

        // Check conflicts in the same 3x3 square
        int squareStartRow = (targetRow / 3) * 3;
        int squareStartColumn = (targetColumn / 3) * 3;
        for (int row = squareStartRow; row < squareStartRow + 3; ++row) {
            for (int col = squareStartColumn; col < squareStartColumn + 3; ++col) {
                int index = row * 9 + col;
                checkAndAddConflict(index);
            }
        }

    }
}

void TileButton::removeConflict(int tile) {
    auto it = std::find(conflicts.begin(), conflicts.end(), tile);
    if (it != conflicts.end()) {
        conflicts.erase(it);
    }

    nlohmann::json j2 = GameData::storageManager->GetData("options_toggle_hlconflicts");

    if(conflicts.empty()){
        showConflicts = false;
    }else{
        showConflicts = j2["value"];
    }
}

void TileButton::addConflict(int tile) {

    conflicts.push_back(tile);

    nlohmann::json j2 = GameData::storageManager->GetData("options_toggle_hlconflicts");

    if(conflicts.empty()){
        showConflicts = false;
    }else{
        showConflicts = j2["value"];
    }
}

void TileButton::calculateAutoCandidates() {
    autoCandidates.clear();
    
    // Don't calculate for permanent tiles or tiles with values
    if (permanent || (!text.empty() && text != "-1")) {
        return;
    }
    
    // Safety check - make sure parent is valid and has children
    if (!parent || parent->children.empty()) {
        return;
    }
    
    int targetRow = tileNumber / 9;
    int targetColumn = tileNumber % 9;
    
    auto childs = parent->children;
    std::set<int> usedNumbers;
    
    // Check row for used numbers
    for (int i = targetRow * 9; i < (targetRow + 1) * 9; ++i) {
        if (i >= 0 && i < static_cast<int>(childs.size())) {
            if (auto* tb = dynamic_cast<TileButton*>(childs[i])) {
                if (!tb->text.empty() && tb->text != "-1") {
                    try {
                        usedNumbers.insert(std::stoi(tb->text));
                    } catch (...) {}
                }
            }
        }
    }
    
    // Check column for used numbers
    for (int i = targetColumn; i < 81; i += 9) {
        if (i >= 0 && i < static_cast<int>(childs.size())) {
            if (auto* tb = dynamic_cast<TileButton*>(childs[i])) {
                if (!tb->text.empty() && tb->text != "-1") {
                    try {
                        usedNumbers.insert(std::stoi(tb->text));
                    } catch (...) {}
                }
            }
        }
    }
    
    // Check 3x3 square for used numbers
    int squareStartRow = (targetRow / 3) * 3;
    int squareStartColumn = (targetColumn / 3) * 3;
    for (int row = squareStartRow; row < squareStartRow + 3; ++row) {
        for (int col = squareStartColumn; col < squareStartColumn + 3; ++col) {
            int index = row * 9 + col;
            if (index >= 0 && index < static_cast<int>(childs.size())) {
                if (auto* tb = dynamic_cast<TileButton*>(childs[index])) {
                    if (!tb->text.empty() && tb->text != "-1") {
                        try {
                            usedNumbers.insert(std::stoi(tb->text));
                        } catch (...) {}
                    }
                }
            }
        }
    }
    
    // Add all numbers 1-9 that aren't used and weren't manually removed
    for (int i = 1; i <= 9; ++i) {
        if (usedNumbers.find(i) == usedNumbers.end()) {
            // Check if this candidate was manually removed
            if (std::find(manuallyRemovedCandidates.begin(), manuallyRemovedCandidates.end(), i) == manuallyRemovedCandidates.end()) {
                autoCandidates.push_back(i);
            }
        }
    }
}

std::vector<int> TileButton::getAutoCandidates() {
    return autoCandidates;
}

void TileButton::updateAutoCandidateDisplay() {
    // This method can be called when the auto candidate setting is toggled
    // to force a recalculation and redraw
    calculateAutoCandidates();
}

void TileButton::SetReplayMode(bool overrideEnabled, bool autoMode) {
    replayOverrideMode = overrideEnabled;
    replayAutoMode = autoMode;
}

