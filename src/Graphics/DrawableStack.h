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
    void AddDrawable(Drawable* drawable);
    void RemoveDrawable(Drawable* drawable);
private:
    std::vector<Drawable*> m_Drawables = {};
};


#endif //RLSUDOKU_DRAWABLESTACK_H
