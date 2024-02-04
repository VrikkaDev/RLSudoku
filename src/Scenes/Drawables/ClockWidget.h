//
// Created by VrikkaDev on 1.2.2024.
//

#ifndef RLSUDOKU_CLOCKWIDGET_H
#define RLSUDOKU_CLOCKWIDGET_H


#include "Graphics/Drawable.h"

class ClockWidget : public Drawable{
public:
    ClockWidget();
    explicit ClockWidget(Rectangle rec);
    void OnStart() override;

    void Draw() override;

    Color color = GRAY;
    Color hoverColor = DARKGRAY;
    Color textColor = WHITE;

    bool isFinished = false;
    bool paused = false;

    int fontSize = 40;

private:
    double startTime = 0.0;
    double currentTime = 0.0;
    double previousTime = 0.0;
};


#endif //RLSUDOKU_CLOCKWIDGET_H
