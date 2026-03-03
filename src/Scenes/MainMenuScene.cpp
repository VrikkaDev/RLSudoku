//
// Created by VrikkaDev on 29.1.2024.
//

#include "MainMenuScene.h"
#include "Scenes/Drawables/GenericButton.h"
#include "GameData.h"
#include "OptionsScene.h"
#include "LeaderboardScene.h"
#include "Scenes/Drawables/GenericDropdown.h"
#include "GameScene.h"
#include "Storage/StorageManager.h"
#include "Helpers/TimeHelper.h"
#include "StatisticsScene.h"
#include "Scenes/Drawables/TextWidget.h"
#include "Storage/RemoteSyncManager.h"

#include <cmath>

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

    // Leaderboard button
    float lw = 280, lh = 50, lx = GetScreenWidth()/2 - lw/2, ly = oy + oh + 10;
    auto lb = new GenericButton("Leaderboards", Rectangle{lx,ly,lw,lh});
    lb->OnClick = [](MouseEvent* event) {
        GameData::SetScene(std::make_unique<LeaderboardScene>());
    };
    drawableStack->AddDrawable(lb);

    // Statistics button
    float sw = 240, sh = 50, sx = GetScreenWidth()/2 - sw/2, sy = ly + lh + 10;
    auto sb = new GenericButton("Statistics", Rectangle{sx,sy,sw,sh});
    sb->OnClick = [](MouseEvent* event) {
        GameData::SetScene(std::make_unique<StatisticsScene>());
    };
    drawableStack->AddDrawable(sb);

    // Quit button
    float qw = 190, qh = 50, qx = GetScreenWidth()/2 - qw/2, qy = sy + sh + 10;
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

    int statusY = static_cast<int>(GetScreenHeight() - 36);
    syncStatusText = new TextWidget("Sync: Connecting...", 16, statusY, 20, DARKGRAY, false);
    drawableStack->AddDrawable(syncStatusText);

}

void MainMenuScene::OnUpdate() {
    Scene::OnUpdate();

    if (!syncStatusText) {
        return;
    }

    if (GameData::remoteSyncManager) {
        auto state = GameData::remoteSyncManager->GetConnectionState();
        if (state == RemoteSyncManager::ConnectionState::Connecting) {
            int dotCount = static_cast<int>(std::fmod(GetTime() * 2.0, 4.0));
            syncStatusText->text = std::string("Sync: Connecting") + std::string(dotCount, '.');
        } else {
            syncStatusText->text = GameData::remoteSyncManager->GetConnectionStatusText();
        }

        switch (state) {
            case RemoteSyncManager::ConnectionState::Connected:
                syncStatusText->color = GREEN;
                break;
            case RemoteSyncManager::ConnectionState::Connecting:
                syncStatusText->color = DARKGRAY;
                break;
            case RemoteSyncManager::ConnectionState::NotConfigured:
            case RemoteSyncManager::ConnectionState::Disabled:
            case RemoteSyncManager::ConnectionState::Error:
            default:
                syncStatusText->color = MAROON;
                break;
        }
    } else {
        syncStatusText->text = "Sync: Unavailable";
        syncStatusText->color = MAROON;
    }
}

void MainMenuScene::OnResize() {
    GameData::SetScene(std::make_unique<MainMenuScene>());
}

MainMenuScene::~MainMenuScene() = default;
