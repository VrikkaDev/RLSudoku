//
// Created by VrikkaDev on 29.1.2024.
//

#ifndef RLSUDOKU_MOUSEEVENT_H
#define RLSUDOKU_MOUSEEVENT_H


#include "Event.h"

class MouseEvent : public Event{
public:
    MouseEvent() : Event() {
    };
    MouseEvent(int et, int mb, Vector2 mp): Event(), EventType(et), MouseButton(mb), MousePosition(mp) {}
    ~MouseEvent() = default;
    int EventType = 0; // 0-NONE, 1-CLICKED, 2-RELEASED
    int MouseButton = 0; // Button state: 0-NORMAL, 1-LEFT_CLICK, 2-RIGHT_CLICK
    Vector2 MousePosition {0,0};
};


#endif //RLSUDOKU_MOUSEEVENT_H
