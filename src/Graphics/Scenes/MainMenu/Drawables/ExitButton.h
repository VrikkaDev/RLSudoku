//
// Created by VrikkaDev on 29.1.2024.
//

#ifndef RLSUDOKU_EXITBUTTON_H
#define RLSUDOKU_EXITBUTTON_H


#include "Graphics/Drawable.h"

class ExitButton : public Drawable{
public:
    ExitButton();
    void Draw() override;
    void OnClick(MouseEvent* event) override;
};


#endif //RLSUDOKU_EXITBUTTON_H
