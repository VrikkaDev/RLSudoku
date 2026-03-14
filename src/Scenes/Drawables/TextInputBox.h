// Simple text input widget for in-game UI filtering.

#ifndef RLSUDOKU_TEXTINPUTBOX_H
#define RLSUDOKU_TEXTINPUTBOX_H

#include "Graphics/Drawable.h"
#include <functional>
#include <string>

class TextInputBox : public Drawable {
public:
    TextInputBox(const char* initialText, Rectangle rec);

    void Draw() override;
    void OnUpdate() override;
    void OnStart() override;

    std::string text;
    std::string placeholder = "Search...";
    int maxLength = 32;
    int fontSize = 20;

    Color bgColor = GRAY;
    Color borderColor = DARKGRAY;
    Color focusedBorderColor = BLUE;
    Color textColor = WHITE;
    Color placeholderColor = LIGHTGRAY;

    bool focused = false;
    float caretBlinkTimer = 0.0f;
    bool caretVisible = true;

    std::function<void(const std::string&)> OnChanged;
};

#endif //RLSUDOKU_TEXTINPUTBOX_H
