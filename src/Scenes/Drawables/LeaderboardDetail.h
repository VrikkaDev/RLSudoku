//
// Created for RLSudoku Leaderboard Detail Panel
//

#ifndef RLSUDOKU_LEADERBOARDDETAIL_H
#define RLSUDOKU_LEADERBOARDDETAIL_H

#include "Graphics/Drawable.h"
#include "Storage/LeaderboardManager.h"

class LeaderboardDetail : public Drawable {
public:
    LeaderboardDetail(Rectangle rec);
    
    void Draw() override;
    
    const LeaderboardEntry* entry = nullptr;
    
private:
    float lineHeight = 22;
};

#endif //RLSUDOKU_LEADERBOARDDETAIL_H
