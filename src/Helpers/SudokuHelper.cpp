//
// Created by VrikkaDev on 1.2.2024.
//

#include "SudokuHelper.h"

using namespace sudoku;

static const unsigned TIMEOUT_SECS = 1200U;

sudoku::GeneratorResult SudokuHelper::GenerateSudoku(sudoku::PuzzleDifficulty difficulty, sudoku::Board& generatedBoard) {
    GeneratorResult result = GeneratorResult::NoError;
    std::atomic<bool> finished{false};
    Generator gen;

    auto asyncGenFinished = [&generatedBoard, &result, &finished](
            GeneratorResult genResult,
            const Board &genBoard) {
        result = genResult;
        generatedBoard = genBoard;
        finished = true;
    };

    auto asyncGenProgress = [](uint8_t currentStep, uint8_t totalSteps) {
        std::cout << "Performing generating step '" << static_cast<int>(currentStep)
             << "' of '" << static_cast<int>(totalSteps) << "'..." << std::endl;
    };

    auto startTime = std::chrono::system_clock::now();
    std::cout << "Generating board with difficulty level '"
         << static_cast<int>(difficulty) << "' ..." << std::endl;

    result = gen.asyncGenerate(difficulty, asyncGenProgress, asyncGenFinished);
    if (result != GeneratorResult::AsyncGenSubmitted) {
        return result;
    }

    unsigned int numOfWaits = 0;
    while (!finished && numOfWaits < TIMEOUT_SECS) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        numOfWaits++;
    }

    if (numOfWaits < TIMEOUT_SECS) {
        auto stopTime = std::chrono::system_clock::now();

        std::cout << "... generated in "
             << std::chrono::duration_cast<std::chrono::milliseconds>(stopTime -
                                                            startTime)
                     .count()
             << " milliseconds:\n"
             << generatedBoard
             << "\nblanks:" << (int)generatedBoard.blankPositionCount()
             << "\nmaxBlank allowed:"
             << (int)Generator::maxEmptyPositions(difficulty) << std::endl;
    } else {
        // Timed-out: cancel
        gen.cancelAsyncGenerate();
    }

    return result;
}
