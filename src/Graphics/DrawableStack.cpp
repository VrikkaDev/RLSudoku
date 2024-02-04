//
// Created by VrikkaDev on 29.1.2024.
//

#include "DrawableStack.h"
#include "Drawable.h"
#include "Event/MouseEvent.h"

void DrawableStack::Draw() {
    // Loops through drawables and draws them
    for (const auto& drawable : m_Drawables){
        if(drawable){
            drawable->Draw();
        }
    }
}

void DrawableStack::OnEvent(Event* event) {
    for (const auto& drawable : m_Drawables){

        if (!drawable){
            continue;
        }

        drawable->OnEvent(event);

        // Handle sub-events
        if (auto* me = dynamic_cast<MouseEvent*>(event)){
            if (!CheckCollisionPointRec(me->MousePosition, drawable->GetRectangle())){
                continue;
            }

            // EventType 2 is RELEASED
            if (me->EventType == 2){
                drawable->OnClick(me);
            }
        }


    }
}

void DrawableStack::OnUpdate() {
    for (const auto& drawable : m_Drawables) {

        if (!drawable) {
            continue;
        }

        drawable->OnUpdate();

        for (auto children : drawable->children){
            children->OnUpdate();
        }
    }
}

void DrawableStack::AddDrawable(Drawable* drawable) {
    m_Drawables.push_back(drawable);
    drawable->OnStart();
}

void DrawableStack::RemoveDrawable(Drawable* drawable) {
    auto it = std::find(m_Drawables.begin(), m_Drawables.end(), drawable);
    if (it != m_Drawables.end()) {
        m_Drawables.erase(it);
    }
}

bool DrawableStack::IsHovering(Vector2 mousePosition) {
    for (const auto& drawable : m_Drawables) {
        if(drawable->IsHovering(mousePosition)){
            return true;
        }
    }
    return false;
}
