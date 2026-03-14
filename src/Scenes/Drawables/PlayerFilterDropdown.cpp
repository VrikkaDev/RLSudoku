#include "PlayerFilterDropdown.h"

#include "Helpers/TextHelper.h"
#include "Helpers/UIHelper.h"

#include <algorithm>

PlayerFilterDropdown* PlayerFilterDropdown::openDropdown = nullptr;

PlayerFilterDropdown::PlayerFilterDropdown(const Rectangle& rec) : Drawable() {
    x = static_cast<int>(rec.x);
    y = static_cast<int>(rec.y);
    width = static_cast<int>(rec.width);
    height = static_cast<int>(rec.height);

    OnSelectionChanged = [](int, const std::string&) {};
}

void PlayerFilterDropdown::OnStart() {
    OnClick = [this](MouseEvent* event) {
        if (!event || event->EventType != 2) {
            return;
        }
        isOpen = !isOpen;
        openDropdown = isOpen ? this : nullptr;
        event->StopPropagation();
    };

    OnEvent = [this](Event* event) {
        auto* me = dynamic_cast<MouseEvent*>(event);
        if (!me || me->EventType != 2 || !isOpen) {
            return;
        }

        if (!IsPointInsideOpenDropdown(me->MousePosition)) {
            isOpen = false;
            if (openDropdown == this) {
                openDropdown = nullptr;
            }
            return;
        }

        me->StopPropagation();

        const Rectangle panelRect = GetPanelRectangle();
        if (CheckCollisionPointRec(me->MousePosition, panelRect)) {
            const int visibleCount = GetVisibleCount();
            const float rowY = me->MousePosition.y - panelRect.y;
            const int rowIndex = static_cast<int>(rowY / static_cast<float>(height));

            if (rowIndex >= 0 && rowIndex < visibleCount) {
                const int itemIndex = scrollStartIndex + rowIndex;
                if (itemIndex >= 0 && itemIndex < static_cast<int>(items.size())) {
                    SetSelectedIndex(itemIndex, true);
                }
            }
            isOpen = false;
            if (openDropdown == this) {
                openDropdown = nullptr;
            }
            return;
        }
    };
}

void PlayerFilterDropdown::OnUpdate() {
    if (openDropdown == this && !isOpen) {
        openDropdown = nullptr;
    }

    if (!isOpen) {
        return;
    }

    const Rectangle panelRect = GetPanelRectangle();
    if (!CheckCollisionPointRec(GetMousePosition(), panelRect)) {
        return;
    }

    if (items.size() <= static_cast<size_t>(maxVisibleItems)) {
        return;
    }

    const float wheel = GetMouseWheelMove();
    if (wheel > 0.0f) {
        scrollStartIndex = std::max(0, scrollStartIndex - 1);
    } else if (wheel < 0.0f) {
        const int maxStart = static_cast<int>(items.size()) - maxVisibleItems;
        scrollStartIndex = std::min(maxStart, scrollStartIndex + 1);
    }
}

void PlayerFilterDropdown::Draw() {
    const bool darkMode = UIHelper::IsDarkModeEnabled();

    Color topColor = bgColor;
    Color topHover = hoverColor;
    Color topPress = pressColor;
    Color topText = textColor;

    if (darkMode) {
        if (topColor.r == GRAY.r && topColor.g == GRAY.g && topColor.b == GRAY.b && topColor.a == GRAY.a) {
            topColor = CLITERAL(Color){55, 55, 62, 255};
        }
        if (topHover.r == DARKGRAY.r && topHover.g == DARKGRAY.g && topHover.b == DARKGRAY.b && topHover.a == DARKGRAY.a) {
            topHover = CLITERAL(Color){75, 75, 84, 255};
        }
        if (topPress.r == LIGHTGRAY.r && topPress.g == LIGHTGRAY.g && topPress.b == LIGHTGRAY.b && topPress.a == LIGHTGRAY.a) {
            topPress = CLITERAL(Color){95, 95, 106, 255};
        }
        if (topText.r == WHITE.r && topText.g == WHITE.g && topText.b == WHITE.b && topText.a == WHITE.a) {
            topText = CLITERAL(Color){230, 230, 236, 255};
        }
    }

    const bool isHovering = CheckCollisionPointRec(GetMousePosition(), GetRectangle());
    const bool isPressed = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

    DrawRectangle(x, y, width, height, isHovering ? (isPressed ? topPress : topHover) : topColor);
    DrawRectangleLines(x, y, width, height, borderColor);

    std::string selected = GetSelectedText();
    if (selected.size() > 18) {
        selected = selected.substr(0, 18);
    }
    std::string topTextValue = labelPrefix + selected + (isOpen ? "  ^" : "  v");

    const int fittedTopFont = GetFittedFontSize(topTextValue.c_str(), fontSize, 10, static_cast<float>(width - 10));
    DrawTextBC(topTextValue.c_str(), x, y, fittedTopFont, width, height, topText);

    if (!isOpen) {
        return;
    }

    const Rectangle panelRect = GetPanelRectangle();
    DrawRectangleRec(panelRect, panelColor);
    DrawRectangleLines(static_cast<int>(panelRect.x), static_cast<int>(panelRect.y), static_cast<int>(panelRect.width), static_cast<int>(panelRect.height), borderColor);

    const int visibleCount = GetVisibleCount();
    for (int i = 0; i < visibleCount; ++i) {
        const int itemIndex = scrollStartIndex + i;
        if (itemIndex < 0 || itemIndex >= static_cast<int>(items.size())) {
            continue;
        }

        const float rowY = panelRect.y + static_cast<float>(i * height);
        const Rectangle rowRect{panelRect.x, rowY, panelRect.width, static_cast<float>(height)};
        const bool rowHovered = CheckCollisionPointRec(GetMousePosition(), rowRect);

        Color rowColor = ColorAlpha(BLACK, 0.0f);
        if (itemIndex == selectedIndex) {
            rowColor = selectedRowColor;
        } else if (rowHovered) {
            rowColor = ColorAlpha(topHover, 0.85f);
        }
        DrawRectangleRec(rowRect, rowColor);

        std::string itemText = items[itemIndex];
        if (itemText.size() > 24) {
            itemText = itemText.substr(0, 24);
        }

        const int fittedRowFont = GetFittedFontSize(itemText.c_str(), fontSize, 10, panelRect.width - 12.0f);
        DrawTextBCL(itemText.c_str(), static_cast<int>(panelRect.x + 6), static_cast<int>(rowY), fittedRowFont, height, topText);
    }

    if (items.size() > static_cast<size_t>(maxVisibleItems)) {
        const int maxStart = static_cast<int>(items.size()) - maxVisibleItems;
        const std::string scrollHint = std::to_string(scrollStartIndex + 1) + "/" + std::to_string(maxStart + 1);
        DrawTextBCL(scrollHint.c_str(), static_cast<int>(panelRect.x + panelRect.width - 58), static_cast<int>(panelRect.y - 16), std::max(10, fontSize - 6), 14, topText);
    }
}

void PlayerFilterDropdown::SetItems(const std::vector<std::string>& values) {
    items = values;
    ClampState();
}

void PlayerFilterDropdown::SetSelectedIndex(int index, bool notify) {
    if (items.empty()) {
        selectedIndex = 0;
        scrollStartIndex = 0;
        return;
    }

    const int oldIndex = selectedIndex;
    selectedIndex = std::max(0, std::min(index, static_cast<int>(items.size()) - 1));

    if (selectedIndex < scrollStartIndex) {
        scrollStartIndex = selectedIndex;
    }

    const int visibleCount = GetVisibleCount();
    if (visibleCount > 0 && selectedIndex >= scrollStartIndex + visibleCount) {
        scrollStartIndex = selectedIndex - visibleCount + 1;
    }

    ClampState();

    if (notify && selectedIndex != oldIndex && OnSelectionChanged) {
        OnSelectionChanged(selectedIndex, items[selectedIndex]);
    }
}

std::string PlayerFilterDropdown::GetSelectedText() const {
    if (items.empty()) {
        return "All";
    }
    if (selectedIndex < 0 || selectedIndex >= static_cast<int>(items.size())) {
        return items.front();
    }
    return items[selectedIndex];
}

Rectangle PlayerFilterDropdown::GetPanelRectangle() const {
    const int visibleCount = GetVisibleCount();
    return Rectangle{
        static_cast<float>(x),
        static_cast<float>(y + height),
        static_cast<float>(width),
        static_cast<float>(height * visibleCount)
    };
}

int PlayerFilterDropdown::GetVisibleCount() const {
    if (items.empty()) {
        return 1;
    }
    const int positiveMax = std::max(1, maxVisibleItems);
    return std::min(static_cast<int>(items.size()), positiveMax);
}

void PlayerFilterDropdown::ClampState() {
    if (items.empty()) {
        selectedIndex = 0;
        scrollStartIndex = 0;
        return;
    }

    selectedIndex = std::max(0, std::min(selectedIndex, static_cast<int>(items.size()) - 1));

    const int maxStart = std::max(0, static_cast<int>(items.size()) - GetVisibleCount());
    scrollStartIndex = std::max(0, std::min(scrollStartIndex, maxStart));
}

bool PlayerFilterDropdown::IsPointInsideOpenDropdown(Vector2 point) const {
    if (!isOpen) {
        return false;
    }
    return CheckCollisionPointRec(point, GetRectangle()) || CheckCollisionPointRec(point, GetPanelRectangle());
}

bool PlayerFilterDropdown::IsPointerOverOpenDropdown(Vector2 point) {
    if (!openDropdown) {
        return false;
    }
    return openDropdown->IsPointInsideOpenDropdown(point);
}
