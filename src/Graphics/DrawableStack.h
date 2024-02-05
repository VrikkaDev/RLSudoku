//
// Created by VrikkaDev on 29.1.2024.
//

#ifndef RLSUDOKU_DRAWABLESTACK_H
#define RLSUDOKU_DRAWABLESTACK_H

#include "pch.hxx"
#include "Event/Event.h"

class Drawable;

class DrawableStack {
public:
    DrawableStack() = default;
    void Draw();
    void OnEvent(Event* event);
    void OnUpdate();
    bool IsHovering(Vector2 mousePosition);
    void AddDrawable(Drawable* drawable);
    void RemoveDrawable(Drawable* drawable);
    std::vector<Drawable*> drawables = {};
private:
};


#endif //RLSUDOKU_DRAWABLESTACK_H
