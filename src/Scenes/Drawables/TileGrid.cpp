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

TileGrid::TileGrid(SudokuBoard* brd, SudokuBoard* solved, Rectangle rec) {
    board = brd;
    solvedBoard = solved;
    x = rec.x;
    y = rec.y;
    width = rec.width;
    height = rec.height;
}

void TileGrid::OnStart() {
    OnClick = [this](MouseEvent* event){
        if (auto* me = dynamic_cast<MouseEvent*>(event)){
            for (auto* dr : children){
                if (!CheckCollisionPointRec(me->MousePosition, dr->GetRectangle())){
                    continue;
                }

                // EventType 2 is RELEASED
                if (me->EventType == 2){
                    dr->OnClick(me);
                }
            }
        }
    };

    OnEvent = [this](Event* event){
        if (auto* ke = dynamic_cast<KeyboardEvent*>(event)){

            if(ke->EventType != 1){
                return;
            }

            int num = KeyHelper::GetNumberFromKeyNum(ke->Key);

            // Set the num of selected tile on keypress
            auto* dr = this->children[this->selectedTile];
            if (auto* drdre = dynamic_cast<TileButton*>(dr)){
                drdre->SetText(std::to_string(num));
            }

            // To refresh "options_toggle_hlsamenumbers"
            SelectTile(selectedTile);
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

            auto val = board->at(t);
            vstr = std::to_string(static_cast<int>(val->value));

            // If its 0 make it be nothing
            if(vstr == "0") vstr = "";

            auto* tb = new TileButton(t, vstr.c_str(), solvedBoard->at(t)->value, rec);
            tb->fontSize=ow/1.5;
            tb->parent = this;

            children.push_back(tb);
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
}

void TileGrid::SelectTile(int tilenumber) {

    selectedTile = tilenumber;

    nlohmann::json j = GameData::storageManager->GetData("options_toggle_showgridlines");
    nlohmann::json j2 = GameData::storageManager->GetData("options_toggle_hlsamenumbers");

    // Determine the row and column of the tile
    int targetRow = tilenumber / 9;
    int targetColumn = tilenumber % 9;

    auto* cb = dynamic_cast<TileButton*>(children[tilenumber]);


    for (auto* d : children){
        if (auto* tb = dynamic_cast<TileButton*>(d)){
            tb->inGridLine = false;
            if (tb->tileNumber == tilenumber){
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

