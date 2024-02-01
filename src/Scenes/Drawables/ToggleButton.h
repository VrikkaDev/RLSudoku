//
// Created by VrikkaDev on 1.2.2024.
//

#ifndef RLSUDOKU_TOGGLEBUTTON_H
#define RLSUDOKU_TOGGLEBUTTON_H


#include "Graphics/Drawable.h"
#include "Storage/Saveable.h"

class ToggleButton : public Drawable, public Saveable{
public:
    explicit ToggleButton(const char* save_token);
    ToggleButton(const char* save_token ,const char* txt, Rectangle rec);
    void Draw() override;
    void OnStart() override;
    nlohmann::json GetJson() override;
    void Load(const nlohmann::json& data) override;

    Color color = GRAY;
    Color hoverColor = DARKGRAY;
    Color pressColor = LIGHTGRAY;
    Color textColor = WHITE;

    int fontSize = 50;

    float togglesymbolSize = 40;
    float togglesymbolRounded = 0.3f;
    int togglesymbolSegments = 20;

    bool value = false;

    std::string text = "Button";
};


#endif //RLSUDOKU_TOGGLEBUTTON_H
