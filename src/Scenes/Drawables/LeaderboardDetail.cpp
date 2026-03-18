//
// Created for RLSudoku Leaderboard Detail Panel
//

#include "LeaderboardDetail.h"
#include "Helpers/TimeHelper.h"
#include "Helpers/TextHelper.h"
#include <sstream>
#include <iomanip>

LeaderboardDetail::LeaderboardDetail(Rectangle rec) : Drawable() {
    x = rec.x;
    y = rec.y;
    width = rec.width;
    height = rec.height;
}

void LeaderboardDetail::Draw() {
    if (!entry) {
        // Draw placeholder
        DrawRectangle(x, y, width, height, ColorAlpha(DARKGRAY, 0.5f));
        DrawRectangleLines(x, y, width, height, GRAY);
        
        const char* msg = "Select an entry";
        const char* msg2 = "to view details";
        int fontSize = 16;
        int textWidth = MeasureText(msg, fontSize);
        DrawTextBCL(msg, x + (width - textWidth) / 2, y + height/2 - fontSize, fontSize, fontSize, GRAY);
        textWidth = MeasureText(msg2, fontSize);
        DrawTextBCL(msg2, x + (width - textWidth) / 2, y + height/2 + 5, fontSize, fontSize, GRAY);
        return;
    }
    
    // Draw panel background
    DrawRectangle(x, y, width, height, ColorAlpha(DARKGRAY, 0.8f));
    DrawRectangleLines(x, y, width, height, WHITE);
    
    float textX = x + 15;
    float textY = y + 15;
    
    // Title
    DrawTextBCL("Run Details", textX, textY, 20, 20, WHITE);
    textY += lineHeight * 1.8f;
    
    // Time
    std::string timeStr = std::string("Time: ") + TimeHelper::GetTimeFormatted(entry->completionTime, true);
    DrawTextBCL(timeStr.c_str(), textX, textY, 16, 16, WHITE);
    textY += lineHeight;
    
    // Difficulty
    std::string diffStr = "Difficulty: " + std::to_string(entry->difficulty);
    DrawTextBCL(diffStr.c_str(), textX, textY, 16, 16, WHITE);
    textY += lineHeight * 1.5f;
    
    // Assists used
    DrawTextBCL("Assists Used:", textX, textY, 16, 16, YELLOW);
    textY += lineHeight;
    
    std::string assistsStr = "  ";
    if (entry->usedAutoCandidates) assistsStr += "Auto-Candidates, ";
    if (entry->usedAutoCheck) assistsStr += "Auto-Check, ";
    if (entry->usedConflictHighlight) assistsStr += "Highlight Conflicts";
    
    if (assistsStr == "  ") {
        assistsStr = "  None";
    } else if (assistsStr.back() == ' ') {
        assistsStr = assistsStr.substr(0, assistsStr.length() - 2);
    }
    
    DrawTextBCL(assistsStr.c_str(), textX, textY, 14, 14, WHITE);
    textY += lineHeight * 1.5f;
    
    // Start and End Times (real-time)
    std::string startTimeStr = "Started: " + TimeHelper::FormatDateTime(entry->startedAt);
    DrawTextBCL(startTimeStr.c_str(), textX, textY, 14, 14, LIGHTGRAY);
    textY += lineHeight;
    
    std::string endTimeStr = "Finished: " + TimeHelper::FormatDateTime(entry->completedAt);
    DrawTextBCL(endTimeStr.c_str(), textX, textY, 14, 14, LIGHTGRAY);
    textY += lineHeight * 1.5f;
    
    // Replay instructions and move count
    std::string movesStr = "Moves: " + std::to_string(entry->moves.size());
    DrawTextBCL(movesStr.c_str(), textX, textY, 16, 16, WHITE);
    textY += lineHeight * 1.5f;
    
    DrawTextBCL("Use 'Watch Replay' or 'Try This Puzzle' on the right.", textX, textY, 14, 14, LIGHTGRAY);
    textY += lineHeight * 1.5f;
    
    // Show first few moves
    DrawTextBCL("First Moves:", textX, textY, 14, 14, LIGHTGRAY);
    textY += lineHeight;
    
    int moveCount = std::min(8, static_cast<int>(entry->moves.size()));
    for (int i = 0; i < moveCount; ++i) {
        const auto& move = entry->moves[i];
        std::stringstream ss;
        ss << "  " << std::fixed << std::setprecision(1) << move.timestamp << "s: ";
        ss << "Tile " << move.tileNumber << " = " << move.value;
        
        if (textY + lineHeight > y + height - 10) {
            DrawTextBCL("  ...", textX, textY, 12, 12, GRAY);
            break;
        }
        
        DrawTextBCL(ss.str().c_str(), textX, textY, 12, 12, GRAY);
        textY += lineHeight * 0.9f;
    }
}
