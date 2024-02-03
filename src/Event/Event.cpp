//
// Created by VrikkaDev on 29.1.2024.
//

#include "Event.h"

void EventDispatcher::AddEvent(Event* event) {
    events.push_back(event);
}
