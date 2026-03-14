//
// Created by VrikkaDev on 29.1.2024.
//

#ifndef RLSUDOKU_EVENT_H
#define RLSUDOKU_EVENT_H


class Event {
public:
    Event() = default;
    virtual ~Event() = default;

    void StopPropagation() { handled = true; }
    [[nodiscard]] bool IsHandled() const { return handled; }

private:
    bool handled = false;
};

class EventDispatcher{
public:
    EventDispatcher() = default;

    void AddEvent(Event* event);

    std::deque<Event*> events{};
};


#endif //RLSUDOKU_EVENT_H
