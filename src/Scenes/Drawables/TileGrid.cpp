//
// Created by VrikkaDev on 31.1.2024.
//

#include "TileGrid.h"
#include "TileButton.h"
#include "Event/KeyboardEvent.h"
#include "Helpers/KeyHelper.h"

TileGrid::TileGrid() : Drawable(){
}

TileGrid::TileGrid(sudoku::Board* brd, sudoku::Solver* slvr, Rectangle rec) {
    solver = slvr;
    board = brd;
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

            auto* dr = this->children[this->selectedTile];
            if (auto* drdre = dynamic_cast<TileButton*>(dr)){
                drdre->SetText(std::to_string(num));
            }
        }
    };

    // Create tiles
    int t = 0;
    for (int i = 0; i < 9; i++){ // Column
        for (int j = 0; j < 9; j++){ // Row

            // Get if dividable by 3 for thicker lines
            float xit = !(i%3) * 2;
            float xjt = !(j%3) * 2;

            float ow = (width/9), oh = (height/9);
            auto rec = Rectangle {(float)x + i * ow + xit, (float)y + j * oh + xjt, ow, oh};

            auto val = board->valueAt(j, i);
            std::string vstr = std::to_string(val);
            if(vstr == "0") vstr = "";

            auto* tb = new TileButton(t, vstr.c_str(), 0, rec);
            tb->fontSize=ow/1.5;
            tb->OnStart();
            tb->parent = this;

            children.push_back(tb);
            t++;
        }
    }
}

void TileGrid::Draw() {
    DrawRectangle(x, y, width, height, color);

    // Draw children aswell
    for (Drawable* tg : children){
        tg->Draw();
    }
}

void TileGrid::SelectTile(int tilenumber) {

    selectedTile = tilenumber;

    for (auto* d : children){
        if (auto* tb = dynamic_cast<TileButton*>(d)){
            if (tb->tileNumber == tilenumber){
                continue;
            }

            tb->DeSelect();
        }
    }
}

