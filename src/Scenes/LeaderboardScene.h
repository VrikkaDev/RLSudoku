//
// Created for RLSudoku Leaderboard Display
//

#ifndef RLSUDOKU_LEADERBOARDSCENE_H
#define RLSUDOKU_LEADERBOARDSCENE_H

#include "Scene.h"
#include "Storage/LeaderboardManager.h"
#include <vector>

class LeaderboardList;
class LeaderboardDetail;
class GenericDropdown;
class GenericButton;
class TextInputBox;
class PlayerFilterDropdown;

class LeaderboardScene : public Scene {
public:
    LeaderboardScene();
    
    void Setup() override;
    void OnResize() override;
    void OnUpdate() override;
    
private:
    int selectedDifficultyMin = 0;
    int selectedDifficultyMax = 100;
    int lastDropdownSelection = 0;
    
    // Assist filter flags
    bool showAutoCandidates = true;
    bool showAutoCheck = true;
    bool showConflictHighlight = true;
    std::vector<std::string> playerFilters;
    int selectedPlayerFilter = 0;
    std::string playerSearchQuery;
    
    LeaderboardList* leaderboardList = nullptr;
    LeaderboardDetail* detailPanel = nullptr;
    GenericDropdown* difficultyDropdown = nullptr;
    GenericButton* filterAutoCandidatesBtn = nullptr;
    GenericButton* filterAutoCheckBtn = nullptr;
    GenericButton* filterConflictHighlightBtn = nullptr;
    PlayerFilterDropdown* playerFilterDropdown = nullptr;
    TextInputBox* playerSearchInput = nullptr;
    GenericButton* replayButton = nullptr;
    GenericButton* tryPuzzleButton = nullptr;
    
    void RefreshEntries();
    void UpdateDetailPanel();
};

#endif //RLSUDOKU_LEADERBOARDSCENE_H
