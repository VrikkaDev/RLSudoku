//
// Created by VrikkaDev on 3.2.2024.
//

#include "WinScreen.h"
#include "Event/GameEvent.h"

WinScreen::WinScreen() : Drawable(){

}

WinScreen::WinScreen(Rectangle rec) {
    x = rec.x;
    y = rec.y;
    width = rec.width;
    height = rec.height;
}

void WinScreen::OnStart() {
    enabled = false;
    OnEvent = [this](Event* event){
        if(auto* ge = dynamic_cast<GameEvent*>(event)){
            // On OnWin enable this
            if(ge->EventType == 1){
                enabled = true;
            }else if(ge->EventType == 2){ // On TimeData set the timer text
                timestring = ge->Data;
            }
        }
    };
}

void WinScreen::Draw() {
    // If enabled draw
    if(enabled){
        DrawRectangle(x, y, width, height, color);
        DrawTextBC("You Won!", x, y-50, 50, width, height, DARKGRAY);
        if(!timestring.empty()){
            DrawTextBC(timestring.c_str(), x, y, 50, width, height, DARKGRAY);
        }
    }
}
