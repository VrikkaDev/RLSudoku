//
// Created for RLSudoku Static Text Display
//

#ifndef RLSUDOKU_TEXTWIDGET_H
#define RLSUDOKU_TEXTWIDGET_H

#include "Graphics/Drawable.h"
#include <string>

class TextWidget : public Drawable {
public:
    std::string text;
    int fontSize;
    Color color;
    bool centered;
    
    TextWidget(const std::string& text, int x, int y, int fontSize = 20, Color color = WHITE, bool centered = false);
    
    void Draw() override;
};

#endif //RLSUDOKU_TEXTWIDGET_H
