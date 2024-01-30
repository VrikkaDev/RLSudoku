//
// Created by VrikkaDev on 29.1.2024.
//

#ifndef RLSUDOKU_DRAWABLE_H
#define RLSUDOKU_DRAWABLE_H

#include "Event/Event.h"
#include "pch.hxx"
#include "Event/MouseEvent.h"

class Drawable{
public:
    Drawable();
    //virtual ~Drawable() = default;
    virtual void Draw(){
    }
    std::function<void(MouseEvent* event)> OnClick;
    std::function<void(Event* event)> OnEvent;

    void AddChild(){
        // TODO children system.... maybe
    }
    void RemoveChild(){

    }

    [[nodiscard]] Rectangle GetRectangle() const{
        auto rec = Rectangle {};
        rec.x = (float)x;
        rec.y = (float)y;
        rec.width = (float)width;
        rec.height = (float)height;
        return rec;
    }

    // Data
    int x = 0, y = 0;
    int width = 100, height = 100;

    std::shared_ptr<Drawable> parent{};
    std::vector<std::shared_ptr<Drawable>> children{};


};

#endif //RLSUDOKU_DRAWABLE_H
