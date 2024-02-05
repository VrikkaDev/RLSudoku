//
// Created by VrikkaDev on 31.1.2024.
//

#include "OptionsScene.h"
#include "Event/MouseEvent.h"
#include "Scenes/Drawables/GenericButton.h"
#include "GameData.h"
#include "MainMenuScene.h"
#include "Storage/StorageManager.h"
#include "Scenes/Drawables/ConfigToggleButton.h"

OptionsScene::OptionsScene() : Scene(){

}

void OptionsScene::Setup() {
    // Back button
    float bw = 250, bh = 50, bx = GetScreenWidth()/2 - bw/2, by = GetScreenHeight() - 70;

    auto bb = new GenericButton("Back", Rectangle{bx,by,bw,bh});
    bb->OnClick = [](MouseEvent* event) {
        // Change scene
        GameData::SetScene(std::make_unique<MainMenuScene>());
    };
    drawableStack->AddDrawable(bb);

    // Highlight conflicts option togglebutton
    float ttw = 230, tth = 50, ttx = 20, tty = GetScreenHeight() - tth*4;
    auto ttb = new ConfigToggleButton("options_toggle_hlconflicts", "Highlight conflicts", Rectangle{ttx, tty, ttw, tth});
    ttb->tooltip = "Enabling this option highlights \nconflicting numbers on the Sudoku grid.";
    ttb->fontSize = 20;
    ttb->defaultValue = true;
    drawableStack->AddDrawable(ttb);

    // Auto check option togglebutton
    float tt2w = 170, tt2h = 50, tt2x = 20, tt2y = GetScreenHeight() - tt2h*5;
    auto tt2b = new ConfigToggleButton("options_toggle_autocheck", "Auto check", Rectangle{tt2x, tt2y, tt2w, tt2h});
    tt2b->tooltip = "When enabled, correct values will be confirmed, \nand conflicts will be highlighted for easy identification.";
    tt2b->fontSize = 20;
    drawableStack->AddDrawable(tt2b);

    // Show clock option togglebutton
    float tsw = 170, tsh = 50, tsx = 20, tsy = GetScreenHeight() - tsh*6;
    auto tsb = new ConfigToggleButton("options_toggle_showclock", "Show clock", Rectangle{tsx, tsy, tsw, tsh});
    tsb->tooltip = "When enabled, clock will be shown on the bottom right of the screen.";
    tsb->fontSize = 20;
    tsb->defaultValue = true;
    drawableStack->AddDrawable(tsb);

    // Show grid lines option togglebutton
    float tglw = 200, tglh = 50, tglx = 20, tgly = GetScreenHeight() - tglh*7;
    auto tglb = new ConfigToggleButton("options_toggle_showgridlines", "Show grid lines", Rectangle{tglx, tgly, tglw, tglh});
    tglb->tooltip = "Toggle the visibility of grid lines to customize the appearance of the Sudoku board. ";
    tglb->fontSize = 20;
    drawableStack->AddDrawable(tglb);

    // Highlight Same Numbers option togglebutton
    float hllw = 280, hllh = 50, hllx = 20, hlly = GetScreenHeight() - hllh*8;
    auto hllb = new ConfigToggleButton("options_toggle_hlsamenumbers", "Highlight Same Numbers", Rectangle{hllx, hlly, hllw, hllh});
    hllb->tooltip = "Enabling this option highlights all occurrences \nof the same number on the Sudoku board.";
    hllb->fontSize = 20;
    drawableStack->AddDrawable(hllb);

}
