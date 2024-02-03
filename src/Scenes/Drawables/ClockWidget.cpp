//
// Created by VrikkaDev on 1.2.2024.
//

#include "ClockWidget.h"
#include "GameData.h"
#include "Storage/StorageManager.h"
#include "Event/GameEvent.h"
#include "Scenes/Scene.h"

ClockWidget::ClockWidget() : Drawable() {

}

ClockWidget::ClockWidget(Rectangle rec) {
    x = rec.x;
    y = rec.y;
    width = rec.width;
    height = rec.height;
}

void ClockWidget::OnStart() {
    startTime = GetTime();

    OnEvent = [this](Event* event){
        if(auto* ge = dynamic_cast<GameEvent*>(event)){
            if(ge->EventType == 1){
                isRunning = false;

                std::chrono::seconds sec((int)currentTime);
                // Convert seconds into hh:mm:ss
                auto seconds = std::to_string(sec.count() % 60);
                if(seconds.size() == 1) seconds = "0"+seconds;
                auto minutes = std::to_string(std::chrono::duration_cast<std::chrono::minutes>(sec).count() % 60);
                if(minutes.size() == 1) minutes = "0"+minutes;
                auto hours = std::to_string(std::chrono::duration_cast<std::chrono::hours>(sec).count());
                if(hours.size() == 1) hours = "0"+hours;

                std::string tx = hours + ":" + minutes
                                 + ":" + seconds;

                // EventType 2 = timedata
                auto* te = new GameEvent(2, tx);
                GameData::currentScene->eventDispatcher->AddEvent(te);
            }
        }
    };
}

void ClockWidget::Draw() {

    nlohmann::json j = GameData::storageManager->GetData("options_toggle_showclock");

    // Don't draw if its not enabled in settings
    if(!j.contains("value") || !j["value"]){
        return;
    }

    // Get the elapsed time in seconds
    double timeNow = GetTime();
    if (isRunning)currentTime = timeNow - startTime;
    std::chrono::seconds sec((int)currentTime);
    // Convert seconds into hh:mm:ss
    auto seconds = std::to_string(sec.count() % 60);
    if(seconds.size() == 1) seconds = "0"+seconds;
    auto minutes = std::to_string(std::chrono::duration_cast<std::chrono::minutes>(sec).count() % 60);
    if(minutes.size() == 1) minutes = "0"+minutes;
    auto hours = std::to_string(std::chrono::duration_cast<std::chrono::hours>(sec).count());
    if(hours.size() == 1) hours = "0"+hours;

    std::string tx = hours + ":" + minutes
            + ":" + seconds;

    DrawRectangle(x, y, width, height, color);
    DrawTextBC(tx.c_str(),x, y, fontSize, width, height, textColor);
}
