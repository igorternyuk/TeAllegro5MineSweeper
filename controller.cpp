#include "controller.h"
#include "model.h"

Controller::Controller(Model *model):
    pModel_(model)
{}

void Controller::leftClick(int x, int y)
{
    pModel_->openCell(x,y);
}

void Controller::rightClick(int x, int y)
{
    pModel_->markCell(x, y);
}

void Controller::newGame()
{
    pModel_->startNewGame();
}

void Controller::changeLevel(int level)
{
    pModel_->changeLevel(level);
}
