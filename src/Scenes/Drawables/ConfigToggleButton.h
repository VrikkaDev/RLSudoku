//
// Created by VrikkaDev on 1.2.2024.
//

#ifndef RLSUDOKU_CONFIGTOGGLEBUTTON_H
#define RLSUDOKU_CONFIGTOGGLEBUTTON_H


#include "Graphics/Drawable.h"
#include "Storage/Saveable.h"

class ConfigToggleButton : public Drawable, public Saveable{
public:
    explicit ConfigToggleButton(const char* save_token);
    ConfigToggleButton(const char* save_token , const char* txt, Rectangle rec);
    void Draw() override;
    void OnStart() override;
    nlohmann::json GetJson() override;
    void Load(const nlohmann::json& data) override;

    Color color = GRAY;
    Color hoverColor = DARKGRAY;
    Color pressColor = LIGHTGRAY;
    Color textColor = WHITE;

    int fontSize = 50;
    int tooltipFontSize = 20;

    float togglesymbolSize = 40;
    float togglesymbolRounded = 0.3f;
    int togglesymbolSegments = 20;

    bool value = false;

    std::string tooltip;
    std::string text = "Button";
private:
    double timeStartHover = 0; // time when started hovering. used for showing tooltip
};


#endif //RLSUDOKU_CONFIGTOGGLEBUTTON_H
