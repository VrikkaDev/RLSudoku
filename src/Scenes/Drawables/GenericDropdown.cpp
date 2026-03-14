//
// Created by VrikkaDev on 31.1.2024.
//

#include "GenericDropdown.h"
#include "Helpers/UIHelper.h"
#include "Scenes/Scene.h"

GenericDropdown::GenericDropdown() : Drawable(), Saveable("generic_dropdown"){
    width = 300;
    height = 50;

    x = GetScreenWidth()/2 - width/2;
    y = GetScreenHeight()/2 - height/2;
}

GenericDropdown::GenericDropdown(std::map<const char*, int> txts, const char* save_token, Rectangle rec) : Drawable(),
                                                                                                           Saveable(save_token){
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
            const bool darkMode = UIHelper::IsDarkModeEnabled();
            for (int i = 1; i <= texts.size(); i++) {
                auto r = Rectangle{(float)x, (float)y + height * i, (float)width, (float)height};
                // Get the correct text
                auto it = texts.begin();
                std::advance(it, i-1);
                auto* gb = new GenericButton(it->first, r);
                gb->parent = this;
                gb->fontSize = fontSize;
                gb->color = darkMode ? CLITERAL(Color){55, 55, 62, 255} : GRAY;
                gb->hoverColor = darkMode ? CLITERAL(Color){75, 75, 84, 255} : DARKGRAY;
                gb->pressColor = darkMode ? CLITERAL(Color){95, 95, 106, 255} : LIGHTGRAY;
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
    if (UIHelper::IsDarkModeEnabled()) {
        if (color.r == GRAY.r && color.g == GRAY.g && color.b == GRAY.b && color.a == GRAY.a) {
            color = CLITERAL(Color){55, 55, 62, 255};
        }
        if (hoverColor.r == DARKGRAY.r && hoverColor.g == DARKGRAY.g && hoverColor.b == DARKGRAY.b && hoverColor.a == DARKGRAY.a) {
            hoverColor = CLITERAL(Color){75, 75, 84, 255};
        }
        if (pressColor.r == LIGHTGRAY.r && pressColor.g == LIGHTGRAY.g && pressColor.b == LIGHTGRAY.b && pressColor.a == LIGHTGRAY.a) {
            pressColor = CLITERAL(Color){95, 95, 106, 255};
        }
        if (triangleColor.r == WHITE.r && triangleColor.g == WHITE.g && triangleColor.b == WHITE.b && triangleColor.a == WHITE.a) {
            triangleColor = CLITERAL(Color){230, 230, 236, 255};
        }
    } else {
        if (color.r == 55 && color.g == 55 && color.b == 62 && color.a == 255) {
            color = GRAY;
        }
        if (hoverColor.r == 75 && hoverColor.g == 75 && hoverColor.b == 84 && hoverColor.a == 255) {
            hoverColor = DARKGRAY;
        }
        if (pressColor.r == 95 && pressColor.g == 95 && pressColor.b == 106 && pressColor.a == 255) {
            pressColor = LIGHTGRAY;
        }
        if (triangleColor.r == 230 && triangleColor.g == 230 && triangleColor.b == 236 && triangleColor.a == 255) {
            triangleColor = WHITE;
        }
    }

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
        DrawTextBCL(it->first, x+5, y, fontSize, height, textColor);
    }

}

int GenericDropdown::GetSelectedValue() {
    auto it = texts.begin();
    std::advance(it, selectedText);
    return it->second;
}

