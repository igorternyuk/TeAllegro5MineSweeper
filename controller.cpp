#include "controller.h"
#include "model.h"

Controller::Controller(Model *model):
    _pModel(model)
{}

void Controller::leftClick(int x, int y)
{
    _pModel->openCell(x,y);
}

void Controller::rightClick(int x, int y)
{
    _pModel->markCell(x, y);
}

void Controller::newGame()
{
    _pModel->startNewGame();
}

void Controller::changeLevel(int level)
{
    _pModel->changeLevel(level);
}
