//
// Created by VrikkaDev on 31.1.2024.
//

#include "OptionsScene.h"
#include "Event/MouseEvent.h"
#include "Scenes/Drawables/GenericButton.h"
#include "GameData.h"
#include "MainMenuScene.h"
#include "Storage/StorageManager.h"
#include "Helpers/UIHelper.h"
#include "Scenes/Drawables/ConfigToggleButton.h"
#include "Scenes/Drawables/GenericDropdown.h"

OptionsScene::OptionsScene() : Scene(){

}

void OptionsScene::Setup() {
    // Back button
    float bw = UIHelper::ScaleX(250.0f), bh = UIHelper::ScaleY(50.0f), bx = GetScreenWidth()/2.0f - bw/2.0f, by = GetScreenHeight() - UIHelper::ScaleY(70.0f);

    auto bb = new GenericButton("Back", Rectangle{bx,by,bw,bh});
    bb->fontSize = UIHelper::ScaleFont(50);
    bb->OnClick = [](MouseEvent* event) {
        // Change scene
        GameData::SetScene(std::make_unique<MainMenuScene>());
    };
    drawableStack->AddDrawable(bb);

    // Highlight conflicts option togglebutton
    float ttw = UIHelper::ScaleX(230.0f), tth = UIHelper::ScaleY(50.0f), ttx = UIHelper::ScaleX(20.0f), tty = GetScreenHeight() - tth*4;
    auto ttb = new ConfigToggleButton("options_toggle_hlconflicts", "Highlight conflicts", Rectangle{ttx, tty, ttw, tth});
    ttb->tooltip = "Enabling this option highlights \nconflicting numbers on the Sudoku grid.";
    ttb->fontSize = UIHelper::ScaleFont(20);
    ttb->defaultValue = true;
    drawableStack->AddDrawable(ttb);

    // Auto check option togglebutton
    float tt2w = UIHelper::ScaleX(170.0f), tt2h = UIHelper::ScaleY(50.0f), tt2x = UIHelper::ScaleX(20.0f), tt2y = GetScreenHeight() - tt2h*5;
    auto tt2b = new ConfigToggleButton("options_toggle_autocheck", "Auto check", Rectangle{tt2x, tt2y, tt2w, tt2h});
    tt2b->tooltip = "When enabled, correct values will be confirmed, \nand conflicts will be highlighted for easy identification.";
    tt2b->fontSize = UIHelper::ScaleFont(20);
    drawableStack->AddDrawable(tt2b);

    // Show clock option togglebutton
    float tsw = UIHelper::ScaleX(170.0f), tsh = UIHelper::ScaleY(50.0f), tsx = UIHelper::ScaleX(20.0f), tsy = GetScreenHeight() - tsh*6;
    auto tsb = new ConfigToggleButton("options_toggle_showclock", "Show clock", Rectangle{tsx, tsy, tsw, tsh});
    tsb->tooltip = "When enabled, clock will be shown on the bottom right of the screen.";
    tsb->fontSize = UIHelper::ScaleFont(20);
    tsb->defaultValue = true;
    drawableStack->AddDrawable(tsb);

    // Show grid lines option togglebutton
    float tglw = UIHelper::ScaleX(200.0f), tglh = UIHelper::ScaleY(50.0f), tglx = UIHelper::ScaleX(20.0f), tgly = GetScreenHeight() - tglh*7;
    auto tglb = new ConfigToggleButton("options_toggle_showgridlines", "Show grid lines", Rectangle{tglx, tgly, tglw, tglh});
    tglb->tooltip = "Toggle the visibility of grid lines to customize the appearance of the Sudoku board. ";
    tglb->fontSize = UIHelper::ScaleFont(20);
    drawableStack->AddDrawable(tglb);

    // Highlight Same Numbers option togglebutton
    float hllw = UIHelper::ScaleX(280.0f), hllh = UIHelper::ScaleY(50.0f), hllx = UIHelper::ScaleX(20.0f), hlly = GetScreenHeight() - hllh*8;
    auto hllb = new ConfigToggleButton("options_toggle_hlsamenumbers", "Highlight Same Numbers", Rectangle{hllx, hlly, hllw, hllh});
    hllb->tooltip = "Enabling this option highlights all occurrences \nof the same number on the Sudoku board.";
    hllb->fontSize = UIHelper::ScaleFont(20);
    drawableStack->AddDrawable(hllb);

    // Dark mode option togglebutton
    float dmw = UIHelper::ScaleX(170.0f), dmh = UIHelper::ScaleY(50.0f), dmx = UIHelper::ScaleX(20.0f), dmy = GetScreenHeight() - dmh*10;
    auto dmb = new ConfigToggleButton("options_toggle_darkmode", "Dark mode", Rectangle{dmx, dmy, dmw, dmh});
    dmb->tooltip = "Use a darker app theme for backgrounds and UI controls.";
    dmb->fontSize = UIHelper::ScaleFont(20);
    dmb->defaultValue = false;
    drawableStack->AddDrawable(dmb);

    // Auto Candidates option togglebutton
    float aclw = UIHelper::ScaleX(220.0f), aclh = UIHelper::ScaleY(50.0f), aclx = UIHelper::ScaleX(20.0f), acly = GetScreenHeight() - aclh*9;
    auto aclb = new ConfigToggleButton("options_toggle_autocandidates", "Auto Candidates", Rectangle{aclx, acly, aclw, aclh});
    aclb->tooltip = "Automatically shows possible candidate numbers \nfor each empty cell based on Sudoku rules.";
    aclb->fontSize = UIHelper::ScaleFont(20);
    drawableStack->AddDrawable(aclb);


    
    // Auto Remove Candidates option togglebutton
    float arclw = UIHelper::ScaleX(280.0f), arclh = UIHelper::ScaleY(50.0f), arclx = UIHelper::ScaleX(350.0f), arcly = GetScreenHeight() - arclh*4;
    auto arclb = new ConfigToggleButton("options_toggle_autoremovecandidates", "Auto Remove Candidates", Rectangle{arclx, arcly, arclw, arclh});
    arclb->tooltip = "Automatically removes manual candidates from \nrelated tiles when you place a number.";
    arclb->fontSize = UIHelper::ScaleFont(20);
    drawableStack->AddDrawable(arclb);

    // Hold-drag candidate input option togglebutton
    float hdcw = UIHelper::ScaleX(300.0f), hdch = UIHelper::ScaleY(50.0f), hdcx = UIHelper::ScaleX(350.0f), hdcy = GetScreenHeight() - hdch*6;
    auto hdcb = new ConfigToggleButton("options_toggle_holdcandidate", "Hold-Drag Candidates", Rectangle{hdcx, hdcy, hdcw, hdch});
    hdcb->tooltip = "Hold left mouse and drag across candidate numbers \nto toggle multiple candidates quickly.";
    hdcb->fontSize = UIHelper::ScaleFont(20);
    hdcb->defaultValue = true;
    drawableStack->AddDrawable(hdcb);

    // Candidate input without selecting tile first
    float cwsw = UIHelper::ScaleX(320.0f), cwsh = UIHelper::ScaleY(50.0f), cwsx = UIHelper::ScaleX(350.0f), cwsy = GetScreenHeight() - cwsh*7;
    auto cwsb = new ConfigToggleButton("options_toggle_candidateswithoutselection", "Candidates Without Select", Rectangle{cwsx, cwsy, cwsw, cwsh});
    cwsb->tooltip = "Allow toggling candidates on hovered empty tiles \nwithout selecting the tile first.";
    cwsb->fontSize = UIHelper::ScaleFont(20);
    cwsb->defaultValue = false;
    drawableStack->AddDrawable(cwsb);

    // Prevent mouse-down tile selection from overriding candidate input
    float cpbw = UIHelper::ScaleX(340.0f), cpbh = UIHelper::ScaleY(50.0f), cpbx = UIHelper::ScaleX(350.0f), cpby = GetScreenHeight() - cpbh*8;
    auto cpbb = new ConfigToggleButton("options_toggle_candidatepriority", "Candidate Input Priority", Rectangle{cpbx, cpby, cpbw, cpbh});
    cpbb->tooltip = "When tile-select is Mouse Down, prioritize candidate \nclicks over selecting the tile first.";
    cpbb->fontSize = UIHelper::ScaleFont(20);
    cpbb->defaultValue = true;
    drawableStack->AddDrawable(cpbb);

    // Record leaderboard runs toggle
    float rlbw = UIHelper::ScaleX(320.0f), rlbh = UIHelper::ScaleY(50.0f), rlbx = UIHelper::ScaleX(350.0f), rlby = GetScreenHeight() - rlbh*9;
    auto rlbb = new ConfigToggleButton("options_toggle_recordleaderboards", "Record Leaderboards", Rectangle{rlbx, rlby, rlbw, rlbh});
    rlbb->tooltip = "When disabled, completed runs are not submitted \nto local or remote leaderboards.";
    rlbb->fontSize = UIHelper::ScaleFont(20);
    rlbb->defaultValue = true;
    drawableStack->AddDrawable(rlbb);

    // Tile selection trigger dropdown (mouse down / mouse up / both)
    std::map<const char*, int> tileSelectMap = {
        {"Tile Select: Mouse Down", 0},
        {"Tile Select: Mouse Up", 1},
        {"Tile Select: Both", 2}
    };
    float tsdw = UIHelper::ScaleX(300.0f), tsdh = UIHelper::ScaleY(50.0f), tsdx = UIHelper::ScaleX(350.0f), tsdy = GetScreenHeight() - tsdh*5;
    auto tsdb = new GenericDropdown(tileSelectMap, "options_dropdown_tileselecttrigger", Rectangle{tsdx, tsdy, tsdw, tsdh});
    tsdb->fontSize = UIHelper::ScaleFont(20);

    // Default to mouse-up behavior for fresh installs.
    int defaultIndex = 0;
    for (auto it = tileSelectMap.begin(); it != tileSelectMap.end(); ++it, ++defaultIndex) {
        if (it->second == 1) {
            tsdb->selectedText = defaultIndex;
            break;
        }
    }

    drawableStack->AddDrawable(tsdb);

}

void OptionsScene::OnResize() {
    GameData::SetScene(std::make_unique<OptionsScene>());
}
