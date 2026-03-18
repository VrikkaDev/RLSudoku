//
// Created for RLSudoku Leaderboard List Widget
//

#include "LeaderboardList.h"
#include "Helpers/TimeHelper.h"
#include "Helpers/TextHelper.h"
#include "Helpers/UIHelper.h"
#include "PlayerFilterDropdown.h"
#include <sstream>
#include <iomanip>

LeaderboardList::LeaderboardList(Rectangle rec) : Drawable() {
    x = rec.x;
    y = rec.y;
    width = rec.width;
    height = rec.height;
}

void LeaderboardList::Draw() {
    const int headerFont = std::max(12, UIHelper::ScaleFont(18));
    const int rowFont = std::max(11, UIHelper::ScaleFont(16));
    const int legendFont = std::max(10, UIHelper::ScaleFont(13));
    rowHeight = std::max(28.0f, static_cast<float>(rowFont + 14));

    // Draw background
    DrawRectangle(x, y, width, height, ColorAlpha(DARKGRAY, 0.3f));
    DrawRectangleLines(x, y, width, height, GRAY);
    
    // Handle scroll wheel
    Vector2 mousePos = GetMousePosition();
    Rectangle listRect = {(float)x, (float)y, (float)width, (float)height};
    const bool dropdownCapturesPointer = PlayerFilterDropdown::IsPointerOverOpenDropdown(mousePos);
    if (CheckCollisionPointRec(mousePos, listRect) && !dropdownCapturesPointer) {
        float wheelMove = GetMouseWheelMove();
        if (wheelMove != 0) {
            scrollOffset -= wheelMove * rowHeight;
            
            // Clamp scroll offset
            float maxScroll = std::max(0.0f, entries.size() * rowHeight - (height - (rowHeight * 3.5f))); // header + legend reserve
            scrollOffset = std::max(0.0f, std::min(scrollOffset, maxScroll));
        }
    }
    
    // Column headers
    float headerY = y + 10;
    float textX = x + 10;
    const float contentW = std::max(100.0f, static_cast<float>(width - 20));

    const float colRank = textX + contentW * 0.00f;
    const float colTime = textX + contentW * 0.10f;
    const float colDiff = textX + contentW * 0.24f;
    const float colAssist = textX + contentW * 0.40f;
    const float colPlayer = textX + contentW * 0.53f;
    const float colDate = textX + contentW * 0.82f;

    DrawTextBCL("Rank", colRank, headerY, headerFont, headerFont, LIGHTGRAY);
    DrawTextBCL("Time", colTime, headerY, headerFont, headerFont, LIGHTGRAY);
    DrawTextBCL("Diff", colDiff, headerY, headerFont, headerFont, LIGHTGRAY);
    DrawTextBCL("Ast", colAssist, headerY, headerFont, headerFont, LIGHTGRAY);
    DrawTextBCL("Player", colPlayer, headerY, headerFont, headerFont, LIGHTGRAY);
    DrawTextBCL("Date", colDate, headerY, headerFont, headerFont, LIGHTGRAY);
    
    // Draw entries (without scissor mode for now - will add back later)
    float entryY = headerY + 30 - scrollOffset;
    
    for (size_t i = 0; i < entries.size(); ++i) {
        // Skip entries scrolled above visible area
        if (entryY + rowHeight < headerY + 30) {
            entryY += rowHeight;
            continue;
        }
        
        // Stop if we're out of visible space
        if (entryY > y + height - 60) break;
        
        const auto& entry = entries[i];
        Rectangle rowRect = {(float)(x + 5), entryY, (float)(width - 10), rowHeight};
        
        // Check hover and click
        bool isHovered = CheckCollisionPointRec(mousePos, rowRect);
        
        if (isHovered) {
            DrawRectangleRec(rowRect, ColorAlpha(DARKGRAY, 0.5f));
            
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                selectedIndex = i;
            }
        }
        
        if (selectedIndex == static_cast<int>(i)) {
            DrawRectangleRec(rowRect, ColorAlpha(BLUE, 0.3f));
        }
        
        // Determine text color based on assists
        Color textColor = WHITE;
        if (entry.usedAutoCandidates || entry.usedAutoCheck) {
            textColor = YELLOW;
        }
        
        // Rank
        std::string rankStr = std::to_string(i + 1);
        DrawTextBCL(rankStr.c_str(), colRank, entryY + 5, rowFont, rowFont, textColor);
        
        // Time
        std::string timeStr = TimeHelper::GetTimeFormatted(entry.completionTime);
        DrawTextBCL(timeStr.c_str(), colTime, entryY + 5, rowFont, rowFont, textColor);
        
        // Difficulty
        std::string diffStr = std::to_string(entry.difficulty);
        DrawTextBCL(diffStr.c_str(), colDiff, entryY + 5, rowFont, rowFont, textColor);
        
        // Assists indicator
        std::string assistStr;
        if (entry.usedAutoCandidates) assistStr += "C";
        if (entry.usedAutoCheck) assistStr += "A";
        if (entry.usedConflictHighlight) assistStr += "H";
        if (assistStr.empty()) assistStr = "-";
        DrawTextBCL(assistStr.c_str(), colAssist, entryY + 5, rowFont, rowFont, textColor);

        // Submitter / player
        std::string playerStr = entry.playerName.empty() ? std::string("Unknown") : entry.playerName;
        size_t maxPlayerLen = 14;
        if (width < 650) {
            maxPlayerLen = 10;
        } else if (width < 850) {
            maxPlayerLen = 12;
        }
        if (playerStr.size() > maxPlayerLen) {
            playerStr = playerStr.substr(0, maxPlayerLen);
        }
        DrawTextBCL(playerStr.c_str(), colPlayer, entryY + 5, rowFont, rowFont, textColor);
        
        // Date
        const std::string dateStr = TimeHelper::FormatDate(entry.completedAt, "%d.%m.%y");
        DrawTextBCL(dateStr.c_str(), colDate, entryY + 5, std::max(10, rowFont - 1), rowFont, textColor);
        
        entryY += rowHeight;
    }
    
    // Legend at bottom
    float legendY = y + height - 50;
    DrawTextBCL("Legend: C=AutoCandidates, A=AutoCheck, H=Highlight", x + 10, legendY, legendFont, legendFont, GRAY);
    DrawTextBCL("Yellow=Assisted, White=Clean", x + 10, legendY + std::max(14, legendFont + 4), legendFont, legendFont, GRAY);
}

const LeaderboardEntry* LeaderboardList::GetSelectedEntry() const {
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(entries.size())) {
        return &entries[selectedIndex];
    }
    return nullptr;
}
