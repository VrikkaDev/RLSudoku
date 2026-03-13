//
// Created for RLSudoku Leaderboard Display
//

#include "LeaderboardScene.h"
#include "MainMenuScene.h"
#include "Scenes/Drawables/GenericButton.h"
#include "Scenes/Drawables/GenericDropdown.h"
#include "Scenes/Drawables/ConfigToggleButton.h"
#include "Scenes/Drawables/LeaderboardList.h"
#include "Scenes/Drawables/LeaderboardDetail.h"
#include "Scenes/Drawables/TextWidget.h"
#include "GameData.h"
#include "GameScene.h"
#include "ReplayScene.h"
#include "Storage/RemoteSyncManager.h"

#include <algorithm>

LeaderboardScene::LeaderboardScene() : Scene() {
}

void LeaderboardScene::Setup() {
    float screenW = GetScreenWidth();
    float screenH = GetScreenHeight();
    
    // Title
    auto title = new TextWidget("LEADERBOARDS", screenW / 2, 20, 40, WHITE, true);
    drawableStack->AddDrawable(title);
    
    // Difficulty filter dropdown
    float dropdownW = 220, dropdownH = 50;
    float dropdownX = 20, dropdownY = 80;
    
    std::map<const char*, int> difficultyMap = {
        {"All Difficulties", 0}, 
        {"Easy (0-33)", 1}, 
        {"Medium (34-49)", 2}, 
        {"Hard (50-60)", 3},
        {"Very Hard (61-100)", 4}
    };
    
    difficultyDropdown = new GenericDropdown(difficultyMap, "leaderboard_difficulty_filter", Rectangle{dropdownX, dropdownY, dropdownW, dropdownH});
    difficultyDropdown->fontSize = 20;
    // Add later so draw on top
    //drawableStack->AddDrawable(difficultyDropdown);
    
    // Assist filter buttons - toggle individual assist types
    float filterBtnW = 120, filterBtnH = 50;
    float filterBtnX = dropdownX + dropdownW + 20;
    float filterBtnY = dropdownY;
    float filterBtnGap = 10;
    
    // Auto-Candidates filter button
    filterAutoCandidatesBtn = new GenericButton("Auto-Cand", Rectangle{filterBtnX, filterBtnY, filterBtnW, filterBtnH});
    filterAutoCandidatesBtn->fontSize = 16;
    filterAutoCandidatesBtn->color = DARKBLUE;
    filterAutoCandidatesBtn->OnClick = [this](MouseEvent* event) {
        showAutoCandidates = !showAutoCandidates;
        RefreshEntries();
    };
    drawableStack->AddDrawable(filterAutoCandidatesBtn);
    
    // Auto-Check filter button
    filterBtnX += filterBtnW + filterBtnGap;
    filterAutoCheckBtn = new GenericButton("Auto-Check", Rectangle{filterBtnX, filterBtnY, filterBtnW, filterBtnH});
    filterAutoCheckBtn->fontSize = 16;
    filterAutoCheckBtn->color = DARKGREEN;
    filterAutoCheckBtn->OnClick = [this](MouseEvent* event) {
        showAutoCheck = !showAutoCheck;
        RefreshEntries();
    };
    drawableStack->AddDrawable(filterAutoCheckBtn);
    
    // Conflict Highlight filter button
    filterBtnX += filterBtnW + filterBtnGap;
    filterConflictHighlightBtn = new GenericButton("Conflicts", Rectangle{filterBtnX, filterBtnY, filterBtnW, filterBtnH});
    filterConflictHighlightBtn->fontSize = 16;
    filterConflictHighlightBtn->color = ORANGE;
    filterConflictHighlightBtn->OnClick = [this](MouseEvent* event) {
        showConflictHighlight = !showConflictHighlight;
        RefreshEntries();
    };
    drawableStack->AddDrawable(filterConflictHighlightBtn);

    // Player filter button (cycles through known players)
    float playerFilterW = 360.0f;
    float playerFilterH = 50.0f;
    float playerFilterX = dropdownX;
    float playerFilterY = dropdownY + dropdownH + 10.0f;
    playerFilterBtn = new GenericButton("Player: All", Rectangle{playerFilterX, playerFilterY, playerFilterW, playerFilterH});
    playerFilterBtn->fontSize = 18;
    playerFilterBtn->OnClick = [this](MouseEvent* event) {
        if (playerFilters.empty()) {
            return;
        }
        selectedPlayerFilter = (selectedPlayerFilter + 1) % static_cast<int>(playerFilters.size());
        RefreshEntries();
    };
    drawableStack->AddDrawable(playerFilterBtn);
    
    // Leaderboard list widget
    float listW = screenW * 0.52f;
    float listX = 20;
    float listY = 210;
    float listH = screenH - listY - 20;
    
    leaderboardList = new LeaderboardList(Rectangle{listX, listY, listW, listH});
    drawableStack->AddDrawable(leaderboardList);
    
    // Detail panel widget - adjusted to not overlap with back button
    float detailW = screenW - listX - listW - 260; // Leave space for back button
    float detailH = listH;
    float detailX = listX + listW + 20;
    float detailY = listY;
    
    detailPanel = new LeaderboardDetail(Rectangle{detailX, detailY, detailW, detailH});
    drawableStack->AddDrawable(detailPanel);
    
    // Back button - moved to top right
    float backW = 200, backH = 50;
    float backX = screenW - backW - 20;
    float backY = 20;
    
    auto backBtn = new GenericButton("Back", Rectangle{backX, backY, backW, backH});
    backBtn->fontSize = 25;
    backBtn->OnClick = [](MouseEvent* event) {
        GameData::SetScene(std::make_unique<MainMenuScene>());
    };
    drawableStack->AddDrawable(backBtn);

    float replayY = backY + backH + 10.0f;
    replayButton = new GenericButton("Watch Replay", Rectangle{backX, replayY, backW, backH});
    replayButton->fontSize = 22;
    replayButton->color = Color{60, 60, 60, 255};
    replayButton->hoverColor = Color{60, 60, 60, 255};
    replayButton->pressColor = Color{60, 60, 60, 255};
    replayButton->enabled = false;
    replayButton->OnClick = [this](MouseEvent* event) {
        if (!leaderboardList) {
            return;
        }
        if (const auto* entryPtr = leaderboardList->GetSelectedEntry()) {
            GameData::SetScene(std::make_unique<ReplayScene>(*entryPtr));
        }
    };
    drawableStack->AddDrawable(replayButton);

    float tryY = replayY + backH + 10.0f;
    tryPuzzleButton = new GenericButton("Try This Puzzle", Rectangle{backX, tryY, backW, backH});
    tryPuzzleButton->fontSize = 22;
    tryPuzzleButton->color = Color{60, 60, 60, 255};
    tryPuzzleButton->hoverColor = Color{60, 60, 60, 255};
    tryPuzzleButton->pressColor = Color{60, 60, 60, 255};
    tryPuzzleButton->enabled = false;
    tryPuzzleButton->OnClick = [this](MouseEvent* event) {
        if (!leaderboardList) {
            return;
        }
        if (const auto* entryPtr = leaderboardList->GetSelectedEntry()) {
            GameData::SetScene(std::make_unique<GameScene>(entryPtr->initialBoard, entryPtr->solutionBoard, entryPtr->difficulty, true));
        }
    };
    drawableStack->AddDrawable(tryPuzzleButton);

    drawableStack->AddDrawable(difficultyDropdown);
    
    // Initial load
    RefreshEntries();
}

void LeaderboardScene::OnResize() {
    GameData::SetScene(std::make_unique<LeaderboardScene>());
}

void LeaderboardScene::RefreshEntries() {
    // Get all entries for the selected difficulty (no assisted filtering at this stage)
    auto entries = GameData::leaderboardManager->GetTopEntries(
        selectedDifficultyMin, 
        selectedDifficultyMax, 
        500,
        false // Don't filter assisted at manager level
    );

    std::vector<std::string> nextPlayerFilters;
    nextPlayerFilters.emplace_back("All");
    for (const auto& entry : entries) {
        if (entry.playerName.empty()) {
            continue;
        }
        if (std::find(nextPlayerFilters.begin(), nextPlayerFilters.end(), entry.playerName) == nextPlayerFilters.end()) {
            nextPlayerFilters.push_back(entry.playerName);
        }
    }

    std::string wantedPlayer = "All";
    if (!playerFilters.empty() && selectedPlayerFilter >= 0 && selectedPlayerFilter < static_cast<int>(playerFilters.size())) {
        wantedPlayer = playerFilters[selectedPlayerFilter];
    }
    playerFilters = std::move(nextPlayerFilters);
    auto selectedIt = std::find(playerFilters.begin(), playerFilters.end(), wantedPlayer);
    if (selectedIt != playerFilters.end()) {
        selectedPlayerFilter = static_cast<int>(std::distance(playerFilters.begin(), selectedIt));
    } else {
        selectedPlayerFilter = 0;
    }

    std::string selectedPlayer = "All";
    if (selectedPlayerFilter >= 0 && selectedPlayerFilter < static_cast<int>(playerFilters.size())) {
        selectedPlayer = playerFilters[selectedPlayerFilter];
    }

    if (GameData::remoteSyncManager) {
        if (selectedPlayer == "All") {
            GameData::remoteSyncManager->QueueLeaderboardUpdate();
        } else {
            GameData::remoteSyncManager->QueueLeaderboardRefreshForPlayer(selectedPlayer);
        }
    }
    
    // Filter based on individual assist type toggles
    std::vector<LeaderboardEntry> filtered;
    for (const auto& entry : entries) {
        bool includeEntry = true;
        
        // If entry uses auto-candidates and we're hiding them, exclude
        if (entry.usedAutoCandidates && !showAutoCandidates) {
            includeEntry = false;
        }
        
        // If entry uses auto-check and we're hiding them, exclude
        if (entry.usedAutoCheck && !showAutoCheck) {
            includeEntry = false;
        }
        
        // If entry uses conflict highlight and we're hiding them, exclude
        if (entry.usedConflictHighlight && !showConflictHighlight) {
            includeEntry = false;
        }

        if (selectedPlayer != "All" && entry.playerName != selectedPlayer) {
            includeEntry = false;
        }
        
        if (includeEntry) {
            filtered.push_back(entry);
        }
    }
    
    if (leaderboardList) {
        leaderboardList->entries = filtered;
        leaderboardList->selectedIndex = -1; // Reset selection
        leaderboardList->ResetScroll(); // Reset scroll position
    }
    
    if (detailPanel) {
        detailPanel->entry = nullptr;
    }

    if (replayButton) {
        Color disabledColor{60, 60, 60, 255};
        replayButton->enabled = false;
        replayButton->color = disabledColor;
        replayButton->hoverColor = disabledColor;
        replayButton->pressColor = disabledColor;
    }

    if (tryPuzzleButton) {
        Color disabledColor{60, 60, 60, 255};
        tryPuzzleButton->enabled = false;
        tryPuzzleButton->color = disabledColor;
        tryPuzzleButton->hoverColor = disabledColor;
        tryPuzzleButton->pressColor = disabledColor;
    }
    
    // Update button colors to show active/inactive filters
    if (filterAutoCandidatesBtn) {
        filterAutoCandidatesBtn->color = showAutoCandidates ? BLUE : Color{40, 40, 80, 255};
    }
    if (filterAutoCheckBtn) {
        filterAutoCheckBtn->color = showAutoCheck ? GREEN : Color{40, 80, 40, 255};
    }
    if (filterConflictHighlightBtn) {
        filterConflictHighlightBtn->color = showConflictHighlight ? ORANGE : Color{120, 60, 20, 255};
    }

    if (playerFilterBtn) {
        playerFilterBtn->text = std::string("Player: ") + selectedPlayer;
    }
}

void LeaderboardScene::UpdateDetailPanel() {
    const LeaderboardEntry* selectedEntry = nullptr;
    if (leaderboardList) {
        selectedEntry = leaderboardList->GetSelectedEntry();
    }

    if (detailPanel) {
        detailPanel->entry = selectedEntry;
    }

    if (replayButton) {
        if (selectedEntry) {
            replayButton->enabled = true;
            replayButton->color = DARKBLUE;
            replayButton->hoverColor = BLUE;
            replayButton->pressColor = Color{0, 82, 172, 255};
        } else {
            Color disabledColor{60, 60, 60, 255};
            replayButton->enabled = false;
            replayButton->color = disabledColor;
            replayButton->hoverColor = disabledColor;
            replayButton->pressColor = disabledColor;
        }
    }

    if (tryPuzzleButton) {
        if (selectedEntry) {
            tryPuzzleButton->enabled = true;
            tryPuzzleButton->color = DARKGREEN;
            tryPuzzleButton->hoverColor = LIME;
            tryPuzzleButton->pressColor = Color{0, 160, 0, 255};
        } else {
            Color disabledColor{60, 60, 60, 255};
            tryPuzzleButton->enabled = false;
            tryPuzzleButton->color = disabledColor;
            tryPuzzleButton->hoverColor = disabledColor;
            tryPuzzleButton->pressColor = disabledColor;
        }
    }
}

void LeaderboardScene::OnUpdate() {
    Scene::OnUpdate();
    
    // Check if dropdown selection changed
    if (difficultyDropdown) {
        int currentSelection = difficultyDropdown->selectedText;
        if (currentSelection != lastDropdownSelection) {
            lastDropdownSelection = currentSelection;
            
            // Map dropdown value to difficulty range
            auto it = difficultyDropdown->texts.begin();
            std::advance(it, currentSelection);
            int diffValue = it->second;
            
            switch(diffValue) {
                case 0: selectedDifficultyMin = 0; selectedDifficultyMax = 100; break;   // All
                case 1: selectedDifficultyMin = 0; selectedDifficultyMax = 33; break;    // Easy (20)
                case 2: selectedDifficultyMin = 34; selectedDifficultyMax = 49; break;   // Medium (40)
                case 3: selectedDifficultyMin = 50; selectedDifficultyMax = 59; break;   // Hard (50)
                case 4: selectedDifficultyMin = 60; selectedDifficultyMax = 100; break;  // Very Hard (60)
            }
            
            RefreshEntries();
        }
    }
    
    // Update detail panel when selection changes
    UpdateDetailPanel();
}
