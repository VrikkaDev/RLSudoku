#include "TextInputBox.h"

#include "Helpers/TextHelper.h"
#include "Helpers/UIHelper.h"

TextInputBox::TextInputBox(const char* initialText, Rectangle rec) : Drawable() {
    text = initialText ? initialText : "";
    x = static_cast<int>(rec.x);
    y = static_cast<int>(rec.y);
    width = static_cast<int>(rec.width);
    height = static_cast<int>(rec.height);
    OnChanged = [](const std::string&) {};
}

void TextInputBox::OnStart() {
    OnClick = [this](MouseEvent* event) {
        if (!event || event->EventType != 2) {
            return;
        }
        focused = CheckCollisionPointRec(event->MousePosition, GetRectangle());
    };
}

void TextInputBox::OnUpdate() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        focused = CheckCollisionPointRec(GetMousePosition(), GetRectangle());
    }

    if (!focused) {
        caretBlinkTimer = 0.0f;
        caretVisible = true;
        return;
    }

    caretBlinkTimer += GetFrameTime();
    if (caretBlinkTimer >= 0.5f) {
        caretBlinkTimer = 0.0f;
        caretVisible = !caretVisible;
    }

    bool changed = false;

    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key <= 126 && static_cast<int>(text.size()) < maxLength) {
            text.push_back(static_cast<char>(key));
            changed = true;
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !text.empty()) {
        text.pop_back();
        changed = true;
    }

    if (changed) {
        OnChanged(text);
    }
}

void TextInputBox::Draw() {
    if (UIHelper::IsDarkModeEnabled()) {
        if (bgColor.r == GRAY.r && bgColor.g == GRAY.g && bgColor.b == GRAY.b && bgColor.a == GRAY.a) {
            bgColor = CLITERAL(Color){50, 50, 58, 255};
        }
        if (borderColor.r == DARKGRAY.r && borderColor.g == DARKGRAY.g && borderColor.b == DARKGRAY.b && borderColor.a == DARKGRAY.a) {
            borderColor = CLITERAL(Color){95, 95, 110, 255};
        }
        if (placeholderColor.r == LIGHTGRAY.r && placeholderColor.g == LIGHTGRAY.g && placeholderColor.b == LIGHTGRAY.b && placeholderColor.a == LIGHTGRAY.a) {
            placeholderColor = CLITERAL(Color){170, 170, 184, 255};
        }
    }

    DrawRectangle(x, y, width, height, bgColor);
    DrawRectangleLines(x, y, width, height, focused ? focusedBorderColor : borderColor);

    const std::string shown = text.empty() ? placeholder : text;
    const Color shownColor = text.empty() ? placeholderColor : textColor;
    const int fittedFont = GetFittedFontSize(shown.c_str(), fontSize, 10, static_cast<float>(width - 12));
    DrawTextBCL(shown.c_str(), x + 6, y, fittedFont, height, shownColor);

    if (focused && caretVisible) {
        const float textY = static_cast<float>(y + (height - fittedFont) / 2);
        const float caretTop = textY - 1.0f;
        const float caretBottom = textY + static_cast<float>(fittedFont) + 1.0f;
        float caretX = static_cast<float>(x + 6);
        if (!text.empty()) {
            const Vector2 textSize = MeasureTextEx(Fonts::default_font, text.c_str(), static_cast<float>(fittedFont), 0.0f);
            caretX += textSize.x + 1.0f;
        }
        const float maxCaretX = static_cast<float>(x + width - 6);
        if (caretX > maxCaretX) {
            caretX = maxCaretX;
        }
        DrawLineEx(Vector2{caretX, caretTop}, Vector2{caretX, caretBottom}, 2.0f, textColor);
    }
}
