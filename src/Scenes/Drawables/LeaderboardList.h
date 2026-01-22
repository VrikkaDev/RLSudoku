//
// Created for RLSudoku Leaderboard List Widget
//

#ifndef RLSUDOKU_LEADERBOARDLIST_H
#define RLSUDOKU_LEADERBOARDLIST_H

#include "Graphics/Drawable.h"
#include "Storage/LeaderboardManager.h"
#include <vector>

class LeaderboardList : public Drawable {
public:
    LeaderboardList(Rectangle rec);
    
    void Draw() override;
    
    std::vector<LeaderboardEntry> entries;
    int selectedIndex = -1;
    
    int GetSelectedIndex() const { return selectedIndex; }
    const LeaderboardEntry* GetSelectedEntry() const;
    void ResetScroll() { scrollOffset = 0; }
    
private:
    float rowHeight = 35;
    float scrollOffset = 0;
};

#endif //RLSUDOKU_LEADERBOARDLIST_H
