//
// Created by VrikkaDev on 31.1.2024.
//

#include "TileButton.h"
#include "TileGrid.h"

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
}

void TileButton::Draw() {
    bool isHovering = CheckCollisionPointRec(GetMousePosition(), GetRectangle());
    bool isPressed = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

    // Draw outline
    DrawRectangle(x, y, width, height, selectedColor);
    DrawRectangle(x+1, y+1, width-2, height-2, selected ? isPressed ? pressColor : selectedColor : color);

    if (selected && isHovering && text.empty()){
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
    }else{ // Else draw the notes
        int t = 1;
        // Draw small numbers inside
        for (int i = 0; i < 3; i++){
            for (int j = 0; j < 3; j++){
                auto it = std::find(notes.begin(), notes.end(), t);
                if (it == notes.end()){
                    t++;
                    continue;
                }
                auto trec = Rectangle {(float)x + j * width/3, (float)y + i * height/3, (float)fontSize/2, (float)fontSize/2};
                std::string st = std::to_string(t);
                DrawTextBC(st.c_str(), trec.x, trec.y,
                           fontSize/2, fontSize/2, fontSize/2, selected ? textColor : textColor2);
                t++;
            }
        }
    }

    // Draw big number if it exists
    if (!text.empty() && text != "-1"){
        DrawTextBC(text.c_str(), x, y, fontSize*1.25, width, height, selected ? textColor : permanent ? textColor3 : textColor2);
    }
}

void TileButton::DeSelect() {
    selected = false;
}

void TileButton::SetText(const std::string& str) {
    if(permanent){
        return;
    }
    text = str;
}
