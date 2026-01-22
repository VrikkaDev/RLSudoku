//
// Created for RLSudoku statistics display
//

#ifndef RLSUDOKU_STATISTICSSCENE_H
#define RLSUDOKU_STATISTICSSCENE_H

#include "Scene.h"

class StatisticsScene : public Scene {
public:
    StatisticsScene();

    void Setup() override;
    void OnResize() override;
};

#endif // RLSUDOKU_STATISTICSSCENE_H
