//
// Created by VrikkaDev on 3.2.2024.
//

#ifndef RLSUDOKU_WINSCREEN_H
#define RLSUDOKU_WINSCREEN_H


#include "Graphics/Drawable.h"

class WinScreen : public Drawable {
public:
    WinScreen();
    WinScreen(Rectangle rec);

    void OnStart() override;
    void Draw() override;

    std::string timestring;

    Color color = CLITERAL(Color){ 150, 150, 150, 200 } ;
};


#endif //RLSUDOKU_WINSCREEN_H
