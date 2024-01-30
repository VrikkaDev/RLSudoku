//
// Created by VrikkaDev on 29.1.2024.
//

#include "Drawable.h"
#include "GameData.h"
#include "DrawableStack.h"
#include "Scenes/Scene.h"

Drawable::Drawable() :
OnClick([this](MouseEvent* event) {
}),
OnEvent([this](Event* event) {
})
{
}
