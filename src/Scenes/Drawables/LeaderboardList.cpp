//
// Created for RLSudoku Leaderboard List Widget
//

#include "LeaderboardList.h"
#include "Helpers/TimeHelper.h"
#include "Helpers/TextHelper.h"
#include <sstream>
#include <iomanip>

LeaderboardList::LeaderboardList(Rectangle rec) : Drawable() {
    x = rec.x;
    y = rec.y;
    width = rec.width;
    height = rec.height;
}

void LeaderboardList::Draw() {
    // Draw background
    DrawRectangle(x, y, width, height, ColorAlpha(DARKGRAY, 0.3f));
    DrawRectangleLines(x, y, width, height, GRAY);
    
    // Handle scroll wheel
    Vector2 mousePos = GetMousePosition();
    Rectangle listRect = {(float)x, (float)y, (float)width, (float)height};
    if (CheckCollisionPointRec(mousePos, listRect)) {
        float wheelMove = GetMouseWheelMove();
        if (wheelMove != 0) {
            scrollOffset -= wheelMove * rowHeight;
            
            // Clamp scroll offset
            float maxScroll = std::max(0.0f, entries.size() * rowHeight - (height - 120)); // 120 = header + legend space
            scrollOffset = std::max(0.0f, std::min(scrollOffset, maxScroll));
        }
    }
    
    // Column headers
    float headerY = y + 10;
    float textX = x + 10;
    
    DrawTextBCL("Rank", textX, headerY, 18, 18, LIGHTGRAY);
    DrawTextBCL("Time", textX + 60, headerY, 18, 18, LIGHTGRAY);
    DrawTextBCL("Difficulty", textX + 150, headerY, 18, 18, LIGHTGRAY);
    DrawTextBCL("Assists", textX + 260, headerY, 18, 18, LIGHTGRAY);
    DrawTextBCL("Date", textX + 340, headerY, 18, 18, LIGHTGRAY);
    
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
        DrawTextBCL(rankStr.c_str(), textX, entryY + 5, 18, 18, textColor);
        
        // Time
        std::string timeStr = TimeHelper::GetTimeFormatted(entry.completionTime);
        DrawTextBCL(timeStr.c_str(), textX + 60, entryY + 5, 18, 18, textColor);
        
        // Difficulty
        std::string diffStr = std::to_string(entry.difficulty);
        DrawTextBCL(diffStr.c_str(), textX + 180, entryY + 5, 18, 18, textColor);
        
        // Assists indicator
        std::string assistStr;
        if (entry.usedAutoCandidates) assistStr += "C";
        if (entry.usedAutoCheck) assistStr += "A";
        if (entry.usedConflictHighlight) assistStr += "H";
        if (assistStr.empty()) assistStr = "-";
        DrawTextBCL(assistStr.c_str(), textX + 280, entryY + 5, 18, 18, textColor);
        
        // Date
        char dateStr[32];
        std::tm* timeinfo = std::localtime(&entry.completedAt);
        std::strftime(dateStr, sizeof(dateStr), "%m/%d/%y", timeinfo);
        DrawTextBCL(dateStr, textX + 340, entryY + 5, 16, 16, textColor);
        
        entryY += rowHeight;
    }
    
    // Legend at bottom
    float legendY = y + height - 50;
    DrawTextBCL("Legend: C=AutoCandidates, A=AutoCheck, H=Highlight", x + 10, legendY, 14, 14, GRAY);
    DrawTextBCL("Yellow=Assisted, White=Clean", x + 10, legendY + 18, 14, 14, GRAY);
}

const LeaderboardEntry* LeaderboardList::GetSelectedEntry() const {
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(entries.size())) {
        return &entries[selectedIndex];
    }
    return nullptr;
}
