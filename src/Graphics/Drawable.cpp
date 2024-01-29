//
// Created by VrikkaDev on 29.1.2024.
//

#include "Drawable.h"
#include "GameData.h"
#include "DrawableStack.h"
#include "Scenes/Scene.h"

Drawable::Drawable() {
    // Add this to drawableStack
    /*if(GameData::currentScene){
        GameData::currentScene->drawableStack->AddDrawable(this);
    }else{
        std::cout << "Tried to add drawable to empty scene" << std::endl;
    }*/
}
