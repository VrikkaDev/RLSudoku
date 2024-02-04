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

const char* getTimeFormatted(double currentTime){
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
    return std::move(tx).c_str();
}

void ClockWidget::OnStart() {

    nlohmann::json j = GameData::storageManager->GetData("options_toggle_showclock");

    enabled = j.contains("value") && j["value"];

    startTime = GetTime();

    OnEvent = [this](Event* event){
        if(auto* ge = dynamic_cast<GameEvent*>(event)){
            if(ge->EventType == 1){ // EventType 1 == wingame
                isFinished = true;
                // Send timedata event

                // EventType 2 = timedata
                auto* te = new GameEvent(2, getTimeFormatted(currentTime));
                GameData::currentScene->eventDispatcher->AddEvent(te);
            }else if(ge->EventType == 3){// EventType 3 == pause
                if(ge->Data == "0"){
                    paused = false;
                }
            }
        }
    };

    OnClick = [this](MouseEvent* event){
        paused = !paused;

        // Send pause event
        auto* pe = new GameEvent(3, std::to_string(paused));
        GameData::currentScene->eventDispatcher->AddEvent(pe);
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
    if(paused){
        startTime += (timeNow - previousTime);
    }
    else if (!isFinished){
        currentTime = timeNow - startTime;
    }

    std::string tx = getTimeFormatted(currentTime);

    bool isHovering = CheckCollisionPointRec(GetMousePosition(), GetRectangle());

    DrawRectangle(x, y, width, height, isHovering ? hoverColor : color);
    DrawTextBCL(tx.c_str(),x+5, y, fontSize, height, textColor);

    // Draw pause icon

    if(paused){
        float triangleSize = 20;
        Vector2 triangleMidPoint = {static_cast<float>(x + width - triangleSize - 10),
                                    static_cast<float>(y + height - height/2)};
        DrawTriangle({triangleMidPoint.x - triangleSize/4, y + triangleSize/2},
                     {triangleMidPoint.x - triangleSize/4, y + height - triangleSize/2},
                     {triangleMidPoint.x + triangleSize, triangleMidPoint.y},
                     isHovering ? color : hoverColor);
    }else{
        DrawRectangle(x + width - 35, y+10, 8, height-20, isHovering ? color : hoverColor);
        DrawRectangle(x + width - 20, y+10, 8, height-20, isHovering ? color : hoverColor);
    }

    previousTime = timeNow;
}
