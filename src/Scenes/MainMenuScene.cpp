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
#include "Helpers/UIHelper.h"
#include "StatisticsScene.h"
#include "Scenes/Drawables/TextWidget.h"
#include "Scenes/Drawables/TextInputBox.h"
#include "Storage/RemoteSyncManager.h"

#include <cmath>
#include <algorithm>
#include <cctype>

namespace {
std::string TrimCopy(const std::string& value) {
    size_t start = 0;
    size_t end = value.size();

    while (start < end && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }

    return value.substr(start, end - start);
}

std::string GetConfiguredUsername() {
    if (GameData::remoteSyncManager) {
        const std::string configured = TrimCopy(GameData::remoteSyncManager->GetConfig().username);
        if (!configured.empty()) {
            return configured;
        }
    }

    return "";
}
}

MainMenuScene::MainMenuScene() : Scene() {
}

void MainMenuScene::Setup() {
    const float screenW = static_cast<float>(GetScreenWidth());
    const float screenH = static_cast<float>(GetScreenHeight());

    // Username should be stored only in remote_sync.json.
    if (GameData::storageManager) {
        GameData::storageManager->RemoveData("profile_player_name", true);
    }

    const std::string existingUsername = GetConfiguredUsername();
    requiresPlayerName = existingUsername.empty();

    if (requiresPlayerName) {
        const float panelW = std::min(UIHelper::ScaleX(520.0f), screenW - UIHelper::ScaleX(40.0f));
        const float panelH = UIHelper::ScaleY(220.0f);
        const float panelX = (screenW - panelW) * 0.5f;
        const float panelY = (screenH - panelH) * 0.5f;

        auto* title = new TextWidget("Choose username", static_cast<int>(panelX), static_cast<int>(panelY), UIHelper::ScaleFont(30), WHITE, false);
        drawableStack->AddDrawable(title);

        const float inputY = panelY + UIHelper::ScaleY(56.0f);
        playerNameInput = new TextInputBox("", Rectangle{panelX, inputY, panelW, UIHelper::ScaleY(52.0f)});
        playerNameInput->fontSize = UIHelper::ScaleFont(22);
        playerNameInput->maxLength = 24;
        playerNameInput->placeholder = "Enter username for stats and leaderboards";
        drawableStack->AddDrawable(playerNameInput);

        const float statusY = inputY + UIHelper::ScaleY(58.0f);
        playerNameErrorText = new TextWidget("", static_cast<int>(panelX), static_cast<int>(statusY), UIHelper::ScaleFont(18), MAROON, false);
        drawableStack->AddDrawable(playerNameErrorText);

        const float saveW = UIHelper::ScaleX(190.0f);
        const float saveH = UIHelper::ScaleY(48.0f);
        const float saveX = panelX;
        const float saveY = panelY + panelH - saveH;
        playerNameConfirmButton = new GenericButton("Save Username", Rectangle{saveX, saveY, saveW, saveH});
        playerNameConfirmButton->fontSize = UIHelper::ScaleFont(28);
        playerNameConfirmButton->OnClick = [this](MouseEvent* event) {
            TrySavePlayerName();
        };
        drawableStack->AddDrawable(playerNameConfirmButton);

        const float quitW = UIHelper::ScaleX(160.0f);
        const float quitH = saveH;
        const float quitX = panelX + panelW - quitW;
        auto* quitButton = new GenericButton("Quit", Rectangle{quitX, saveY, quitW, quitH});
        quitButton->fontSize = UIHelper::ScaleFont(26);
        quitButton->OnClick = [](MouseEvent* event) {
            GameData::isRunning = false;
        };
        drawableStack->AddDrawable(quitButton);

        return;
    }

    const float gapY = UIHelper::ScaleY(10.0f);
    const float centerX = screenW * 0.5f;
    const float baseTopY = screenH * 0.5f - UIHelper::ScaleY(25.0f);

    // Options button
    float ow = UIHelper::ScaleX(220.0f), oh = UIHelper::ScaleY(50.0f), ox = centerX - ow/2.0f, oy = baseTopY;
    auto ob = new GenericButton("Options", Rectangle{ox,oy,ow,oh});
    ob->fontSize = UIHelper::ScaleFont(50);
    ob->OnClick = [](MouseEvent* event) {
        GameData::SetScene(std::make_unique<OptionsScene>());
    };
    drawableStack->AddDrawable(ob);

    // Leaderboard button
    float lw = UIHelper::ScaleX(280.0f), lh = UIHelper::ScaleY(50.0f), lx = centerX - lw/2.0f, ly = oy + oh + gapY;
    auto lb = new GenericButton("Leaderboards", Rectangle{lx,ly,lw,lh});
    lb->fontSize = UIHelper::ScaleFont(50);
    lb->OnClick = [](MouseEvent* event) {
        GameData::SetScene(std::make_unique<LeaderboardScene>());
    };
    drawableStack->AddDrawable(lb);

    // Statistics button
    float sw = UIHelper::ScaleX(240.0f), sh = UIHelper::ScaleY(50.0f), sx = centerX - sw/2.0f, sy = ly + lh + gapY;
    auto sb = new GenericButton("Statistics", Rectangle{sx,sy,sw,sh});
    sb->fontSize = UIHelper::ScaleFont(50);
    sb->OnClick = [](MouseEvent* event) {
        GameData::SetScene(std::make_unique<StatisticsScene>());
    };
    drawableStack->AddDrawable(sb);

    // Quit button
    float qw = UIHelper::ScaleX(190.0f), qh = UIHelper::ScaleY(50.0f), qx = centerX - qw/2.0f, qy = sy + sh + gapY;
    auto qb = new GenericButton("Quit", Rectangle{qx,qy,qw,qh});
    qb->fontSize = UIHelper::ScaleFont(50);
    qb->OnClick = [](MouseEvent* event) {
        GameData::isRunning = false;
    };
    drawableStack->AddDrawable(qb);

    // Difficulty dropdown

    std::map<const char*, int> difficultyMap = {{"Easy", 20}, {"Medium", 40}, {"Hard", 50}, {"Very Hard", 60}};

    float dw = UIHelper::ScaleX(200.0f), dh = UIHelper::ScaleY(50.0f);
    float dy = baseTopY - UIHelper::ScaleY(70.0f);
    float dx = centerX + UIHelper::ScaleX(260.0f) - dw/2.0f;
    auto db = new GenericDropdown(difficultyMap,
                                  "menu_difficulty_dropdown", Rectangle{dx,dy,dw,dh});
    db->fontSize = UIHelper::ScaleFont(40);
    drawableStack->AddDrawable(db);

    // Generate game button
    float gw = UIHelper::ScaleX(250.0f), gh = UIHelper::ScaleY(50.0f), gx = centerX - gw/2.0f, gy = dy;
    auto gb = new GenericButton("Generate", Rectangle{gx,gy,gw,gh});
    gb->fontSize = UIHelper::ScaleFont(50);
    gb->OnClick = [db](MouseEvent* event) {
        GameData::SetScene(std::make_unique<GameScene>(db->GetSelectedValue()));
    };
    drawableStack->AddDrawable(gb);

    // Continue game button
    nlohmann::json json = GameData::storageManager->GetData("game_save");
    if(json.contains("difficulty")){
        float cw = UIHelper::ScaleX(260.0f), ch = UIHelper::ScaleY(100.0f), cx = UIHelper::ScaleX(5.0f), cy = dy;
        int val = (int)json["difficulty"];
        auto result = std::find_if(
                difficultyMap.begin(),
                difficultyMap.end(),
                [val](const auto& mo) {return mo.second == val; });
        std::string tim = TimeHelper::GetTimeFormatted((double)json["time"]);
        std::string n = "Continue.\nDifficulty: " + std::string(result->first) + "\nTime: " + tim;
        auto cb = new GenericButton(n.c_str(), Rectangle{cx,cy,cw,ch});
        cb->fontSize = UIHelper::ScaleFont(30);
        cb->OnClick = [](MouseEvent* event) {
            GameData::SetScene(std::make_unique<GameScene>(true));
        };
        drawableStack->AddDrawable(cb);
    }

    int statusY = static_cast<int>(screenH - UIHelper::ScaleY(36.0f));
    syncStatusText = new TextWidget("Sync: Connecting...", static_cast<int>(UIHelper::ScaleX(16.0f)), statusY, UIHelper::ScaleFont(20), DARKGRAY, false);
    drawableStack->AddDrawable(syncStatusText);

    int versionY = statusY - UIHelper::ScaleFont(24);
    versionStatusText = new TextWidget("Version: checking policy...", static_cast<int>(UIHelper::ScaleX(16.0f)), versionY, UIHelper::ScaleFont(18), DARKGRAY, false);
    drawableStack->AddDrawable(versionStatusText);

    float updateW = std::max(UIHelper::ScaleX(180.0f), std::min(UIHelper::ScaleX(240.0f), screenW * 0.26f));
    float updateH = UIHelper::ScaleY(42.0f);
    float updateX = screenW - updateW - UIHelper::ScaleX(16.0f);
    float updateY = static_cast<float>(versionY - UIHelper::ScaleFont(8));
    updateButton = new GenericButton("No Update", Rectangle{updateX, updateY, updateW, updateH});
    updateButton->fontSize = UIHelper::ScaleFont(20);
    updateButton->enabled = false;
    updateButton->color = Color{60, 60, 60, 255};
    updateButton->hoverColor = updateButton->color;
    updateButton->pressColor = updateButton->color;
    updateButton->OnClick = [](MouseEvent* event) {
        if (!GameData::remoteSyncManager) {
            return;
        }

        const std::string url = GameData::remoteSyncManager->GetUpdateDownloadUrl();
        if (!url.empty()) {
            OpenURL(url.c_str());
        }
    };
    drawableStack->AddDrawable(updateButton);

}

void MainMenuScene::OnUpdate() {
    Scene::OnUpdate();

    if (requiresPlayerName) {
        if (IsKeyPressed(KEY_ENTER)) {
            TrySavePlayerName();
        }
        return;
    }

    if (!syncStatusText || !versionStatusText || !updateButton) {
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

        if (!GameData::remoteSyncManager->IsVersionPolicyChecked()) {
            if (state == RemoteSyncManager::ConnectionState::Error || state == RemoteSyncManager::ConnectionState::Connecting) {
                versionStatusText->text = "Server startup can take up to a minute.";
                versionStatusText->color = ORANGE;
            } else {
                versionStatusText->text = "Version: checking policy...";
                versionStatusText->color = DARKGRAY;
            }
            updateButton->enabled = false;
            updateButton->text = "No Update";
            Color disabledColor{60, 60, 60, 255};
            updateButton->color = disabledColor;
            updateButton->hoverColor = disabledColor;
            updateButton->pressColor = disabledColor;
            return;
        }

        const bool required = GameData::remoteSyncManager->IsUpdateRequired();
        const bool available = GameData::remoteSyncManager->HasUpdateAvailable();
        const std::string message = GameData::remoteSyncManager->GetVersionPolicyMessage();
        const std::string downloadUrl = GameData::remoteSyncManager->GetUpdateDownloadUrl();

        if (required) {
            versionStatusText->text = "Update required: " + message;
            versionStatusText->color = MAROON;
            updateButton->text = "Update Required";
            updateButton->enabled = !downloadUrl.empty();
            Color active = Color{160, 40, 40, 255};
            Color hover = Color{200, 60, 60, 255};
            Color disabled = Color{90, 40, 40, 255};
            updateButton->color = updateButton->enabled ? active : disabled;
            updateButton->hoverColor = updateButton->enabled ? hover : disabled;
            updateButton->pressColor = updateButton->enabled ? MAROON : disabled;
        } else if (available) {
            versionStatusText->text = "Update available: " + message;
            versionStatusText->color = ORANGE;
            updateButton->text = "Download Update";
            updateButton->enabled = !downloadUrl.empty();
            Color active = Color{180, 110, 20, 255};
            Color hover = Color{220, 140, 30, 255};
            Color disabled = Color{90, 70, 40, 255};
            updateButton->color = updateButton->enabled ? active : disabled;
            updateButton->hoverColor = updateButton->enabled ? hover : disabled;
            updateButton->pressColor = updateButton->enabled ? ORANGE : disabled;
        } else {
            versionStatusText->text = "Version " + GameData::remoteSyncManager->GetClientVersion() + " is up to date.";
            versionStatusText->color = GREEN;
            updateButton->enabled = false;
            updateButton->text = "Up To Date";
            Color disabledColor{40, 90, 40, 255};
            updateButton->color = disabledColor;
            updateButton->hoverColor = disabledColor;
            updateButton->pressColor = disabledColor;
        }
    } else {
        syncStatusText->text = "Sync: Unavailable";
        syncStatusText->color = MAROON;
        versionStatusText->text = "Version: policy unavailable";
        versionStatusText->color = MAROON;
        updateButton->enabled = false;
        updateButton->text = "No Update";
        Color disabledColor{60, 60, 60, 255};
        updateButton->color = disabledColor;
        updateButton->hoverColor = disabledColor;
        updateButton->pressColor = disabledColor;
    }
}

void MainMenuScene::OnResize() {
    GameData::SetScene(std::make_unique<MainMenuScene>());
}

MainMenuScene::~MainMenuScene() = default;

void MainMenuScene::TrySavePlayerName() {
    if (!playerNameInput || !playerNameErrorText) {
        return;
    }

    const std::string trimmedName = TrimCopy(playerNameInput->text);
    if (trimmedName.empty()) {
        playerNameErrorText->text = "Username cannot be empty.";
        playerNameErrorText->color = MAROON;
        return;
    }

    if (GameData::remoteSyncManager) {
        RemoteSyncConfig cfg = GameData::remoteSyncManager->GetConfig();
        if (cfg.username != trimmedName) {
            cfg.username = trimmedName;
            GameData::remoteSyncManager->UpdateConfig(cfg);
        }
    } else {
        playerNameErrorText->text = "Sync manager unavailable.";
        playerNameErrorText->color = MAROON;
        return;
    }

    GameData::SetScene(std::make_unique<MainMenuScene>());
}
