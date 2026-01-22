//
// Created by VrikkaDev on 1.2.2024.
//

#include "ConfigToggleButton.h"
#include "GameData.h"
#include "Storage/StorageManager.h"

ConfigToggleButton::ConfigToggleButton(const char* save_token) : Drawable(), Saveable(save_token) {

}

ConfigToggleButton::ConfigToggleButton(const char* save_token, const char* txt, Rectangle rec) : Drawable(), Saveable(save_token) {
    text = txt;
    x = rec.x;
    y = rec.y;
    width = rec.width;
    height = rec.height;
    
    // Set default OnClick - can be overridden after construction
    OnClick = [this](MouseEvent* event){
        value = !value;
        GameData::storageManager->Save();
    };
}

void ConfigToggleButton::OnStart() {
    // OnStart is called after the button is added to the drawable stack
    // At this point, if a custom OnClick was set, it's already there
    // We don't need to do anything here anymore
}

void ConfigToggleButton::Draw() {
    bool isHovering = CheckCollisionPointRec(GetMousePosition(), GetRectangle());
    bool isPressed = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

    DrawRectangle(x, y, width, height, isHovering ? isPressed ? pressColor : hoverColor : color);
    // plus is so it isn't too far left
    DrawTextBCL(text.c_str(), x+5, y, fontSize, height, textColor);

    // Calculate the offset from edges
    float dif = height - togglesymbolSize;

    auto riRect = Rectangle {(float)x + width - togglesymbolSize - dif/2, (float)y + dif/2, (float)togglesymbolSize, (float)togglesymbolSize};

    DrawRectangleRoundedLines(riRect, togglesymbolRounded, togglesymbolSegments, 2.f, CLITERAL(Color){ 40, 40, 40, 255 } );

    // Draw the x if value is true
    if(value){
        DrawTextBC("X", riRect.x, riRect.y, togglesymbolSize, riRect.width, riRect.height, BLACK);
    }

    // Draw Tooltip
    if(isHovering){
        // Get the elapsed time in seconds
        double currentTime = GetTime();
        double elapsedTime = currentTime - timeStartHover;

        // Draw tooltip
        if (elapsedTime >= 0.8 && !tooltip.empty()){
            DrawTooltipB(tooltip.c_str(), tooltipFontSize, BLACK, WHITE);
        }
    }else{
        timeStartHover = GetTime();
    }


}

nlohmann::json ConfigToggleButton::GetJson() {
    nlohmann::json json;
    json["value"] = value;
    return json;
}

void ConfigToggleButton::Load(const nlohmann::json& data) {
    if(data.contains("value")){
        value = data["value"];
    }else{
        value = defaultValue;
    }
}
