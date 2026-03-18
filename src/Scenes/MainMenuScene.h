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
    void OnUpdate() override;
    void OnResize() override;

private:
    bool requiresPlayerName = false;
    class TextInputBox* playerNameInput = nullptr;
    class TextWidget* playerNameErrorText = nullptr;
    class GenericButton* playerNameConfirmButton = nullptr;
    class TextWidget* usernameInfoText = nullptr;
    class TextWidget* endpointInfoText = nullptr;
    class TextWidget* offlineModeInfoText = nullptr;
    class TextWidget* recordingInfoText = nullptr;
    class TextWidget* cacheInfoText = nullptr;
    class TextWidget* syncStatusText = nullptr;
    class TextWidget* versionStatusText = nullptr;
    class GenericButton* updateButton = nullptr;

    void TrySavePlayerName();
};


#endif //RLSUDOKU_MAINMENUSCENE_H
