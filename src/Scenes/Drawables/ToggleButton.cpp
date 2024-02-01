//
// Created by VrikkaDev on 1.2.2024.
//

#include "ToggleButton.h"

ToggleButton::ToggleButton(const char* save_token) : Drawable(), Saveable(save_token) {

}

ToggleButton::ToggleButton(const char* save_token, const char* txt, Rectangle rec) : Drawable(), Saveable(save_token) {
    text = txt;
    x = rec.x;
    y = rec.y;
    width = rec.width;
    height = rec.height;
}

void ToggleButton::OnStart() {
    OnClick = [this](MouseEvent* event){
        value = !value;
    };
}

void ToggleButton::Draw() {
    bool isHovering = CheckCollisionPointRec(GetMousePosition(), GetRectangle());
    bool isPressed = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

    DrawRectangle(x, y, width, height, isHovering ? isPressed ? pressColor : hoverColor : color);
    // plus is so it isn't too far left
    DrawTextB(text.c_str(), x+5, y, fontSize, textColor);

    // Calculate the offset from edges
    float dif = height - togglesymbolSize;

    auto riRect = Rectangle {(float)x + width - togglesymbolSize - dif/2, (float)y + dif/2, (float)togglesymbolSize, (float)togglesymbolSize};

    DrawRectangleRoundedLines(riRect, togglesymbolRounded, togglesymbolSegments, 2.f, CLITERAL(Color){ 40, 40, 40, 255 } );

    if(value){
        DrawTextBC("X", riRect.x, riRect.y, togglesymbolSize, riRect.width, riRect.height, BLACK);
    }
}

nlohmann::json ToggleButton::GetJson() {
    nlohmann::json json;
    json["value"] = value;
    return json;
}

void ToggleButton::Load(const nlohmann::json& data) {
    std::cout<<data<<std::endl;
    if(data.contains("value")){
        value = data["value"];
    }
}
