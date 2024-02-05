//
// Created by VrikkaDev on 29.1.2024.
//

#ifndef RLSUDOKU_MAINMENUSCENE_H
#define RLSUDOKU_MAINMENUSCENE_H


#include "Scene.h"

class MainMenuScene : public Scene {
public:
    MainMenuScene();
    ~MainMenuScene();
    void Setup() override;
    void OnResize() override;
};


#endif //RLSUDOKU_MAINMENUSCENE_H
