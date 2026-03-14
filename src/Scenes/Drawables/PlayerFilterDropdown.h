#ifndef RLSUDOKU_PLAYERFILTERDROPDOWN_H
#define RLSUDOKU_PLAYERFILTERDROPDOWN_H

#include "Graphics/Drawable.h"
#include <functional>
#include <string>
#include <vector>

class PlayerFilterDropdown : public Drawable {
public:
    PlayerFilterDropdown(const Rectangle& rec);

    void Draw() override;
    void OnStart() override;
    void OnUpdate() override;

    void SetItems(const std::vector<std::string>& values);
    void SetSelectedIndex(int index, bool notify = false);

    [[nodiscard]] std::string GetSelectedText() const;
    [[nodiscard]] int GetSelectedIndex() const { return selectedIndex; }

    int fontSize = 18;
    int maxVisibleItems = 5;
    std::string labelPrefix = "Player: ";

    Color bgColor = GRAY;
    Color hoverColor = DARKGRAY;
    Color pressColor = LIGHTGRAY;
    Color textColor = WHITE;
    Color panelColor = CLITERAL(Color){45, 45, 52, 255};
    Color selectedRowColor = CLITERAL(Color){42, 94, 173, 255};
    Color borderColor = CLITERAL(Color){95, 95, 110, 255};

    std::function<void(int, const std::string&)> OnSelectionChanged;

    static bool IsPointerOverOpenDropdown(Vector2 point);

private:
    std::vector<std::string> items;
    int selectedIndex = 0;
    bool isOpen = false;
    int scrollStartIndex = 0;

    static PlayerFilterDropdown* openDropdown;

    [[nodiscard]] Rectangle GetPanelRectangle() const;
    [[nodiscard]] int GetVisibleCount() const;
    [[nodiscard]] bool IsPointInsideOpenDropdown(Vector2 point) const;
    void ClampState();
};

#endif // RLSUDOKU_PLAYERFILTERDROPDOWN_H
