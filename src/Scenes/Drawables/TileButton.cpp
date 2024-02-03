//
// Created by VrikkaDev on 31.1.2024.
//

#include "TileButton.h"
#include "TileGrid.h"
#include "GameData.h"
#include "Storage/StorageManager.h"

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
        if (!selected){
            selected = true;
        }

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

                // Check if lmb pressed and toggle not if so
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && isHoveringTrec){
                    auto it = std::find(notes.begin(), notes.end(), t);
                    if (it != notes.end()){
                        notes.erase(it);
                    } else{
                        notes.push_back(t);
                    }
                }

                // Makes the notes invisible if far enough when setting notes
                // Calculate the Euclidean distance between the points
                float distance = Vector2Distance(Vector2 {trec.x, trec.y}, GetMousePosition());
                float alpha = 1.f - (distance / (fontSize/ 1.5));
                alpha = Clamp(alpha, 0.0f, 1.0f);
                alpha *= 255;

                auto col = textColor;

                bool alr = std::find(notes.begin(), notes.end(), t) != notes.end();

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
    }else { // Else draw the notes
        int t = 1;
        // Draw small numbers inside
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                auto it = std::find(notes.begin(), notes.end(), t);
                if (it == notes.end()) {
                    t++;
                    continue;
                }
                auto trec = Rectangle{(float) x + j * width / 3, (float) y + i * height / 3, (float) fontSize / 2,
                                      (float) fontSize / 2};
                std::string st = std::to_string(t);
                DrawTextBC(st.c_str(), trec.x, trec.y,
                           fontSize / 2, fontSize / 2, fontSize / 2, selected ? textColor : textColor2);
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

    nlohmann::json j = GameData::storageManager->GetData("options_toggle_autocheck");

    if(auto* tg = dynamic_cast<TileGrid*>(parent)){
        tg->SetTile(tileNumber, stoi(str));
        tg->CheckIfFinished();
    }


    // AutoCheck
    if (j.contains("value") && j["value"]) {
        int si = stoi(str);
        if (si != correctNum) {
            showIsWrong = true;
            text = str;
            return;
        }

    }

    showIsWrong = false;
    text = str;
    CheckForConflicts();
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

