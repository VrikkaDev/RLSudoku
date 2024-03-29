//
// Created by VrikkaDev on 31.1.2024.
//

#include "GameScene.h"
#include "Scenes/Drawables/GenericButton.h"
#include "GameData.h"
#include "MainMenuScene.h"
#include "Scenes/Drawables/TileButton.h"
#include "Scenes/Drawables/TileGrid.h"
#include "Scenes/Drawables/ClockWidget.h"
#include "Scenes/Drawables/NumberButtons.h"
#include "Storage/StorageManager.h"

#include "JCZSolve.h"
#include "Scenes/Drawables/WinScreen.h"

GameScene::GameScene() : Scene(){
}

std::string getRandomLine(const std::string& filename) {
    std::string line;
    std::string selected;
    std::ifstream file(filename);

    if (file.is_open()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0, 1);

        int n = 0;
        while (std::getline(file, line)) {
            ++n;
            if (dis(gen) < 1.0 / n) {
                selected = line;
            }
        }
        file.close();
    } else {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }

    return std::move(selected);
}

GameScene::GameScene(int difficulty) : Scene(){
    this->difficulty = difficulty;

    std::string final_solve;

    // Define the number of threads you want to use
    const int num_threads = 4;

    // Create a vector to hold the threads
    std::vector<std::thread> threads;


    while (true) {
        if(difficulty > 45){
            if(difficulty == 50){
                std::string st = getRandomLine("./assets/puzzles/hard.txt");
                board = std::make_unique<SudokuBoard>(st.c_str());
            }else if(difficulty == 60){
                std::string st = getRandomLine("./assets/puzzles/hardest.txt");
                board = std::make_unique<SudokuBoard>(st.c_str());
            }
        }else{
            board = std::make_unique<SudokuBoard>(genBoard(difficulty));
        }
        std::string st = board->parser();
        char solvstr[81];
        int corcount = JCZSolver(st.c_str(), solvstr, 2);
        if (corcount <= 1) {
            final_solve = std::string(solvstr);
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }


    orgBoard = std::make_unique<SudokuBoard>(board->parser().c_str());
    solvedBoard = std::make_unique<SudokuBoard>(final_solve.c_str());
}

GameScene::GameScene(bool load) {
    if(load){
        newGame = false;
    }
}

void GameScene::Setup() {

    // Game Clock
    float cw = (GetScreenWidth() - (20 + GetScreenHeight() - 80 + 20))/2,
    ch = GetScreenHeight()/7, cx = 20 + GetScreenHeight() - 80 + 20, cy = GetScreenHeight() - ch - 20;
    auto cb = new ClockWidget(Rectangle{cx,cy,cw,ch});
    drawableStack->AddDrawable(cb);

    gs = new GameSaveable(this);
    // i just dont care :)
    nlohmann::json json = GameData::storageManager->GetData("game_save");
    gs->Load(json);

    // Sudoku Tilegrid
    float tw = GetScreenHeight()-80, th = GetScreenHeight()-80, tx = 20, ty = GetScreenHeight()/2 - th/2;
    auto tb = new TileGrid(board.get(), orgBoard.get(), solvedBoard.get(), startNotes, Rectangle{tx,ty,tw,th});
    tileGrid = tb;
    drawableStack->AddDrawable(tb);

    // Number Buttons
    int wwh = GetScreenWidth() - GetScreenHeight();
    wwh = std::min((float)wwh, cy-ch);
    float nw = wwh, nh = wwh, nx = GetScreenWidth() - nw - 20, ny = 20;
    auto nb = new NumberButtons(Rectangle{nx,ny,nw,nh});
    drawableStack->AddDrawable(nb);

    // Win Screen
    float ww = GetScreenWidth(), wh = GetScreenHeight(), wx = 0, wy = 0;
    auto wb = new WinScreen(Rectangle{wx,wy,ww,wh});
    drawableStack->AddDrawable(wb);

    // Back button
    float bw = cw - 20,
    bh = ch, bx = cx + cw + 10, by = cy;
    auto bb = new GenericButton("Back", Rectangle{bx,by,bw,bh});
    bb->fontSize = bh/2;
    bb->OnClick = [](MouseEvent* event) {
        GameData::SetScene(std::make_unique<MainMenuScene>());
    };
    drawableStack->AddDrawable(bb);
}

void GameScene::OnResize() {
    GameData::SetScene(std::make_unique<GameScene>(true));
}


// -------------------GAMESAVEABLE---------------------

GameSaveable::GameSaveable(GameScene* scene) : Saveable("game_save"){
    this->scene = scene;
}

nlohmann::json GameSaveable::GetJson() {

    if(scene->tileGrid->isFinished){
        return "";
    }

    nlohmann::json json;
    // Save board and solution
    json["tileData"] = scene->board->parser();
    json["orgData"] = scene->orgBoard->parser();
    json["solData"] = scene->solvedBoard->parser();
    json["difficulty"] = scene->difficulty;
    double time = 0.0;
    // Get time from timewidget
    for(const auto& dr : scene->drawableStack->drawables){
        if(auto* cw = dynamic_cast<ClockWidget*>(dr)){
            time = cw->GetCurrentTime();
            break;
        }
    }
    json["time"] = time;

    // Get notes
    std::string notes = {};
    for (const auto& d : scene->tileGrid->children) {
        if (auto* tb = dynamic_cast<TileButton*>(d)) {
            std::string notesstring;
            for(int i : tb->notes){
                if (notesstring.find(std::to_string(i)) == std::string::npos){
                    notesstring += std::to_string(i);
                }
            }
            if(notesstring.empty()){
                continue;
            }
            notes += "|" + std::to_string(tb->tileNumber) + "," + notesstring;
        }
    }
    json["notes"] = notes;
    return json;
}

void GameSaveable::Load(const nlohmann::json& data) {
    if(scene->newGame || data.empty()){
        return;
    }
    scene->newGame = true;

    std::string td = (std::string)data["tileData"];
    std::string od = (std::string)data["orgData"];
    std::string sd = (std::string)data["solData"];

    scene->orgBoard = std::make_unique<SudokuBoard>(od.c_str());
    scene->board = std::make_unique<SudokuBoard>(td.c_str());
    scene->solvedBoard = std::make_unique<SudokuBoard>(sd.c_str());
    scene->difficulty = data["difficulty"];

    std::string ss = data["notes"];

    std::vector<std::string> nots = StringHelper::SplitString(ss.c_str(), '|');

    for(const auto& tx : nots){
        if(tx.empty()){
            continue;
        }

        std::vector<std::string> ton = StringHelper::SplitString(tx.c_str(), ',');
        int ind = stoi(ton[0]);

        scene->startNotes[ind] = ton[1];
    }

    // Set time for timewidget
    for(const auto& dr : scene->drawableStack->drawables){
        if(auto* cw = dynamic_cast<ClockWidget*>(dr)){
            cw->SetTime((double)data["time"]);
            // Pause
            cw->OnClick(nullptr);
        }
    }
}