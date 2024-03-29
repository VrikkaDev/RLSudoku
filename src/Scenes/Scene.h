//
// Created by VrikkaDev on 29.1.2024.
//

#ifndef RLSUDOKU_SCENE_H
#define RLSUDOKU_SCENE_H


#include "Graphics/DrawableStack.h"
#include "Storage/SaveableStack.h"

class Scene {
public:
    Scene();
    ~Scene() = default;
    virtual void Setup(){}
    virtual void OnUpdate();
    virtual void OnResize();

    bool DidSetup = false;
    std::shared_ptr<EventDispatcher> eventDispatcher;
    std::shared_ptr<DrawableStack> drawableStack;
    SaveableStack saveableStack = SaveableStack();
};


#endif //RLSUDOKU_SCENE_H
