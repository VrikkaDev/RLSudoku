//
// Created by VrikkaDev on 31.1.2024.
//

#include "GenericDropdown.h"
#include "GameData.h"
#include "Scenes/Scene.h"

GenericDropdown::GenericDropdown() : Drawable(){
    width = 300;
    height = 50;

    x = GetScreenWidth()/2 - width/2;
    y = GetScreenHeight()/2 - height/2;
}

GenericDropdown::GenericDropdown(std::map<const char*, int> txts, Rectangle rec) : Drawable(){
    texts = txts;
    x = rec.x;
    y = rec.y;
    width = rec.width;
    height = rec.height;
}

void GenericDropdown::OnStart() {
    OnClick = [this](MouseEvent* event){
        isOpened = !isOpened;

        if(isOpened){
            for (int i = 1; i <= texts.size(); i++) {
                auto r = Rectangle{(float)x, (float)y + height * i, (float)width, (float)height};
                // Get the correct text
                auto it = texts.begin();
                std::advance(it, i-1);
                auto* gb = new GenericButton(it->first, r);
                gb->parent = this;
                gb->fontSize = fontSize;
                gb->OnClick = [gb, i](MouseEvent* event){
                    if (auto* gd = dynamic_cast<GenericDropdown*>(gb->parent)){
                        gd->selectedText = i-1;
                        gd->OnClick(event);
                    }
                };
                textButtons.push_back(gb);
            }
        }else{
            textButtons.clear();
        }
    };

    OnEvent = [this](Event* event){
        // Check click event of the dropdown buttons
        for (const auto& gb : textButtons){
            if (auto* me = dynamic_cast<MouseEvent*>(event)){
                if (!CheckCollisionPointRec(me->MousePosition, gb->GetRectangle())){
                    continue;
                }

                // EventType 2 is RELEASED
                if (me->EventType == 2){
                    gb->OnClick(me);
                }
            }
        }

    };
}

void GenericDropdown::Draw() {
    bool isHovering = CheckCollisionPointRec(GetMousePosition(), GetRectangle());
    bool isPressed = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

    DrawRectangle(x, y, width, height, isHovering ? isPressed ? pressColor : hoverColor : color);

    // Calculate the midpoint of the triangle within the rectangle
    Vector2 triangleMidPoint = {static_cast<float>(x + width - triangleSize - 10),
                                static_cast<float>(y + height - height/2)};


    // Draw the triangle upside down if opened
    if(isOpened){
        DrawTriangle({triangleMidPoint.x, triangleMidPoint.y - triangleSize/2},
                     {triangleMidPoint.x - triangleSize, triangleMidPoint.y + triangleSize},
                     {triangleMidPoint.x + triangleSize, triangleMidPoint.y + triangleSize},
                     triangleColor);

        for(GenericButton* b : textButtons){
            b->Draw();
        }

        // Draw line under the top rectangle
        DrawRectangle(x, y+height, width, 2, triangleColor);
    }else{
        // The /2 is to make it be in same position. because of the flattened end of triangle
        DrawTriangle({triangleMidPoint.x + triangleSize, triangleMidPoint.y - triangleSize/2},
                     {triangleMidPoint.x - triangleSize, triangleMidPoint.y - triangleSize/2},
                     {triangleMidPoint.x, triangleMidPoint.y + triangleSize},
                     triangleColor);
    }

    // Draw the top text
    if (texts.size() > selectedText){
        auto it = texts.begin();
        std::advance(it, selectedText);
        DrawTextBC(it->first, x, y, fontSize, width, height, textColor);
    }

}

int GenericDropdown::GetSelectedValue() {
    auto it = texts.begin();
    std::advance(it, selectedText);
    return it->second;
}

