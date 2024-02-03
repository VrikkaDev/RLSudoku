//
// Created by VrikkaDev on 29.1.2024.
//

#include "Scene.h"
#include "GameData.h"

Scene::Scene() {
    drawableStack = std::make_shared<DrawableStack>();
    eventDispatcher = std::make_shared<EventDispatcher>();
}

void Scene::OnUpdate() {
    // Dispatch events
    while (!eventDispatcher->events.empty()){
        auto* event = eventDispatcher->events.front();
        drawableStack->OnEvent(event);
        eventDispatcher->events.pop_front();
    }
}
