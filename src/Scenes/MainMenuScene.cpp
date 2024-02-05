//
// Created by VrikkaDev on 29.1.2024.
//

#include "MainMenuScene.h"
#include "Scenes/Drawables/GenericButton.h"
#include "GameData.h"
#include "OptionsScene.h"
#include "Scenes/Drawables/GenericDropdown.h"
#include "GameScene.h"
#include "Storage/StorageManager.h"
#include "Helpers/TimeHelper.h"

MainMenuScene::MainMenuScene() : Scene() {
}

void MainMenuScene::Setup() {

    // Options button
    float ow = 220, oh = 50, ox = GetScreenWidth()/2 - ow/2, oy = GetScreenHeight()/2 - oh/2;
    auto ob = new GenericButton("Options", Rectangle{ox,oy,ow,oh});
    ob->OnClick = [](MouseEvent* event) {
        GameData::SetScene(std::make_unique<OptionsScene>());
    };
    drawableStack->AddDrawable(ob);

    // Quit button
    float qw = 190, qh = 50, qx = GetScreenWidth()/2 - qw/2, qy = GetScreenHeight()/2 - qh/2 + 70;
    auto qb = new GenericButton("Quit", Rectangle{qx,qy,qw,qh});
    qb->OnClick = [](MouseEvent* event) {
        GameData::isRunning = false;
    };
    drawableStack->AddDrawable(qb);

    // Difficulty dropdown

    std::map<const char*, int> difficultyMap = {{"Easy", 20}, {"Medium", 40}, {"Hard", 50}, {"Very Hard", 60}};

    float dw = 200, dh = 50, dx = GetScreenWidth()/2 - dw/2 + 260, dy = GetScreenHeight()/2 - dh/2 - 70;
    auto db = new GenericDropdown(difficultyMap,
                                  "menu_difficulty_dropdown", Rectangle{dx,dy,dw,dh});
    drawableStack->AddDrawable(db);

    // Generate game button
    float gw = 250, gh = 50, gx = GetScreenWidth()/2 - gw/2, gy = GetScreenHeight()/2 - gh/2 - 70;
    auto gb = new GenericButton("Generate", Rectangle{gx,gy,gw,gh});
    gb->OnClick = [db](MouseEvent* event) {
        GameData::SetScene(std::make_unique<GameScene>(db->GetSelectedValue()));
    };
    drawableStack->AddDrawable(gb);

    // Continue game button
    nlohmann::json json = GameData::storageManager->GetData("game_save");
    if(json.contains("difficulty")){
        float cw = 260, ch = 100, cx = 5, cy = GetScreenHeight()/2 - ch/2 - 70;
        int val = (int)json["difficulty"];
        auto result = std::find_if(
                difficultyMap.begin(),
                difficultyMap.end(),
                [val](const auto& mo) {return mo.second == val; });
        std::string tim = TimeHelper::GetTimeFormatted((double)json["time"]);
        std::string n = "Continue.\nDifficulty: " + std::string(result->first) + "\nTime: " + tim;
        auto cb = new GenericButton(n.c_str(), Rectangle{cx,cy,cw,ch});
        cb->fontSize = 30;
        cb->OnClick = [](MouseEvent* event) {
            GameData::SetScene(std::make_unique<GameScene>(true));
        };
        drawableStack->AddDrawable(cb);
    }

}

void MainMenuScene::OnResize() {
    GameData::SetScene(std::make_unique<MainMenuScene>());
}

MainMenuScene::~MainMenuScene() = default;
