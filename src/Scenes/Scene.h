//
// Created by VrikkaDev on 29.1.2024.
//

#ifndef RLSUDOKU_SCENE_H
#define RLSUDOKU_SCENE_H


#include "Graphics/DrawableStack.h"

class Scene {
public:
    Scene();
    ~Scene() = default;
    virtual void Setup(){}
    virtual void OnUpdate();
    std::shared_ptr<EventDispatcher> eventDispatcher;
    std::shared_ptr<DrawableStack> drawableStack;
};


#endif //RLSUDOKU_SCENE_H
