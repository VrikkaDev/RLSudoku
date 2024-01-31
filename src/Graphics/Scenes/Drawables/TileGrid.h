//
// Created by VrikkaDev on 31.1.2024.
//

#ifndef RLSUDOKU_TILEGRID_H
#define RLSUDOKU_TILEGRID_H


#include "Graphics/Drawable.h"

class TileGrid : public Drawable{
public:
    TileGrid();
    explicit TileGrid(Rectangle rec);
    void Draw() override;
    void OnStart() override;

    void SelectTile(int tilenumber);

    Color color = DARKGRAY;
};


#endif //RLSUDOKU_TILEGRID_H
