//
// Created by VrikkaDev on 31.1.2024.
//

#ifndef RLSUDOKU_OPTIONSSCENE_H
#define RLSUDOKU_OPTIONSSCENE_H


#include "Scene.h"

class OptionsScene : public Scene {
public:
    OptionsScene();

    void Setup() override;
    void OnResize() override;
};


#endif //RLSUDOKU_OPTIONSSCENE_H
