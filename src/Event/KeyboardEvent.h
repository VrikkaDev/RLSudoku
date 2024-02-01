//
// Created by VrikkaDev on 1.2.2024.
//

#ifndef RLSUDOKU_KEYBOARDEVENT_H
#define RLSUDOKU_KEYBOARDEVENT_H


#include "Event.h"
#include "raylib.h"

class KeyboardEvent : public Event{
public:
    KeyboardEvent() : Event() {
    };
    KeyboardEvent(int et, int key) : Event() {
        EventType = et;
        Key = key;
    };
    int EventType = 0; // 0-NONE, 1-PRESSED
    int Key = 0;
};


#endif //RLSUDOKU_KEYBOARDEVENT_H
