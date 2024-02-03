//
// Created by VrikkaDev on 3.2.2024.
//

#ifndef RLSUDOKU_GAMEEVENT_H
#define RLSUDOKU_GAMEEVENT_H

#include "Event.h"

class GameEvent : public Event{
public:
    GameEvent() : Event(){};
    GameEvent(int eventType, std::string data) : Event(){
        EventType = eventType;
        Data = std::move(data);
    };
    ~GameEvent() = default;

    int EventType = 0; // 0:NONE 1:WINGAME 2:TIMEDATA
    std::string Data;
};

#endif //RLSUDOKU_GAMEEVENT_H
