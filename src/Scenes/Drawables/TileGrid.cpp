//
// Created by VrikkaDev on 31.1.2024.
//

#include "TileGrid.h"
#include "TileButton.h"
#include "Event/KeyboardEvent.h"
#include "Helpers/KeyHelper.h"
#include "GameData.h"
#include "Storage/StorageManager.h"
#include "Event/GameEvent.h"
#include "Scenes/Scene.h"

TileGrid::TileGrid() : Drawable(){
}

TileGrid::TileGrid(SudokuBoard* brd, SudokuBoard* orgBrd, SudokuBoard* solved, Rectangle rec) : Drawable() {
    orgBoard = orgBrd;
    board = brd;
    solvedBoard = solved;
    x = rec.x;
    y = rec.y;
    width = rec.width;
    height = rec.height;
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
    }
}

void TileGrid::Draw() {
    DrawRectangle(x, y, width-1, height-1, color);

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
    }
}

void TileGrid::SetTile(int tilenumber, int value) {

    if (value < 0) value = 0;

    tileValues[tilenumber] = value;
    board->setValue(tilenumber, value);
}
