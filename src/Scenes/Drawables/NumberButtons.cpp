//
// Created by VrikkaDev on 1.2.2024.
//

#include "NumberButtons.h"
#include "GenericButton.h"
#include "GameData.h"
#include "Scenes/Scene.h"
#include "Scenes/GameScene.h"
#include "TileGrid.h"
#include "Event/KeyboardEvent.h"

NumberButtons::NumberButtons() : Drawable() {

}

NumberButtons::NumberButtons(Rectangle rec) {
    x = rec.x;
    y = rec.y;
    width = rec.width;
    height = rec.height;
}

void NumberButtons::OnStart() {
    // Create buttons
    int t = 0;
    for (int i = 0; i < 3; i++){ // Column
        for (int j = 0; j < 3; j++){ // Row

            float ow = (width/4), oh = (height/4);
            auto rec = Rectangle {(float)x + ow/2 + j + j  * ow, (float)y + i + i * oh, ow, oh};

            auto* tb = new GenericButton(std::to_string(t+1).c_str(), rec);
            tb->fontSize=ow/1.5;
            tb->OnStart();
            tb->OnClick = [tb,t](MouseEvent* event){
                // Get Gamescene from currentscene
                if (auto* gs = dynamic_cast<GameScene*>(GameData::currentScene.get())){
                    // Create fake keyboardinput event and give it to tilegrid
                    // In raylib 49 is key one
                    auto* ke = new KeyboardEvent(1, 49+t);
                    gs->tileGrid->OnEvent(ke);
                }
            };
            tb->parent = this;

            children.push_back(tb);
            t++;
        }
    }

    float ow = (width/4), oh = (height/4);
    auto rec = Rectangle {(float)x + ow/2, (float)y + oh * 3 , ow*3+2, oh};

    auto* tb = new GenericButton("X", rec);
    tb->fontSize=ow/1.5;
    tb->OnStart();
    tb->OnClick = [](MouseEvent* event){
        // Get Gamescene from currentscene
        if (auto* gs = dynamic_cast<GameScene*>(GameData::currentScene.get())){
            // Create fake keyboardinput event and give it to tilegrid
            auto* ke = new KeyboardEvent(1, KEY_BACKSLASH);
            gs->tileGrid->OnEvent(ke);
        }
    };
    tb->parent = this;

    children.push_back(tb);

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

}

void NumberButtons::Draw() {
    //DrawRectangle(x, y, width, height, color);

    for(auto* dr : children){
        dr->Draw();
    }
}
