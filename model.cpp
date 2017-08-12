#include "model.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "modellistener.h"

Model::Model(int level)
{
    changeLevel(level);
   // mineField(_numMines);
}

void Model::openCell(int cx, int cy)
{
    if(gameState_ == GameState::PLAY && isCellCoordsInRange(cx,cy) &&
       field_[cy][cx].state == CellState::CLOSED)
    {
        field_[cy][cx].state = CellState::OPENED;
        if(field_[cy][cx].isMined)
        {
            gameState_ = GameState::DEFEAT;
            explosionX_ = cx;
            explosionY_ = cy;
            //Show all mines
            for(int y{0}; y < height_; ++y)
            {
                for(int x{0}; x < width_; ++x)
                {
                    if(field_[y][x].isMined && field_[y][x].state != CellState::FLAGGED)
                        field_[y][x].state = CellState::OPENED;
                }
            }
            notifyAllListeners();
            return;
        }
        if(isWin())
        {
            gameState_ = GameState::VICTORY;
            notifyAllListeners();
            return;
        }
        if(isFirstMove_)
        {
            mineField(numMines_);
            isFirstMove_ = false;
        }
        if(countMinesAround(cx, cy) == 0)
        {
            for(int dx{-1}; dx < 2; ++dx)
            {
                for(int dy{-1}; dy < 2; ++dy)
                {
                    int neighbourX = cx + dx;
                    int neighbourY = cy + dy;
                    if(!isCellCoordsInRange(neighbourX, neighbourY) ||
                        (cx == neighbourX && cy == neighbourY))
                        continue;
                    if(field_[neighbourY][neighbourX].state == CellState::CLOSED)
                        openCell(neighbourX, neighbourY);
                }
            }
        }
        notifyAllListeners();
    }
}

void Model::markCell(int x, int y)
{
    if(isCellCoordsInRange(x, y))
    {
        switch(field_[y][x].state)
        {
            case CellState::CLOSED:
                if(numFlags_ < numMines_)
                {
                    ++numFlags_;
                    field_[y][x].state = CellState::FLAGGED;
                }
                else
                {
                    field_[y][x].state = CellState::QUESTIONED;
                }
                break;
            case CellState::FLAGGED:
                --numFlags_;
                field_[y][x].state = CellState::QUESTIONED;
                break;
            case CellState::QUESTIONED:
                field_[y][x].state = CellState::CLOSED;
                break;
            case CellState::OPENED:
                break;
        }
        if(isWin())
        {
            gameState_ = GameState::VICTORY;
        }
        notifyAllListeners();
    }
}

void Model::startNewGame()
{
    changeLevel(level_);
}

void Model::changeLevel(int level)
{
    //std::cout << "level = " << level << std::endl;
    if(level < 1 || level > 3)
        level = 1;
    width_ = levelsData_[level - 1][0];
    height_ = levelsData_[level - 1][1];
    numMines_ = levelsData_[level - 1][2];
    level_ = level;
    numFlags_ = 0;
    isFirstMove_ = true;
    explosionX_ = -1;
    explosionY_ = -1;
    gameState_ = GameState::PLAY;
    createField();
    notifyAllListeners();
}

int Model::countMinesAround(int x, int y)
{
    int counter {0};
    for(int dx{-1}; dx < 2; ++dx)
    {
        for(int dy{-1}; dy < 2; ++dy)
        {
            int neighbourX = x + dx;
            int neighbourY = y + dy;
            if(!isCellCoordsInRange(neighbourX, neighbourY) ||
               (x == neighbourX && y == neighbourY))
                continue;
            if(field_[neighbourY][neighbourX].isMined)
                ++counter;
        }
    }
    return counter;
}

void Model::addListener(ModelListener *listener)
{
    listeners_.push_back(listener);
}

void Model::removeListener(ModelListener *listener)
{
    listeners_.remove(listener);
}

void Model::createField()
{
    for(int y{0}; y < height_; ++y)
    {
        for(int x{0}; x < width_; ++x)
        {
            field_[y][x].state = CellState::CLOSED;
            field_[y][x].isMined = false;
        }
    }
}

void Model::mineField(int numMines)
{
    srand(time(0));
    for(int i{0}; i < numMines; ++i)
    {
        int randX, randY;
        do
        {
            randX = rand() % width_;
            randY = rand() % height_;
        }
        while(field_[randY][randX].isMined || field_[randY][randX].state == CellState::OPENED);
        field_[randY][randX].isMined = true;
    }
    notifyAllListeners();
}

bool Model::isCellCoordsInRange(int x, int y)
{
    return x >= 0 && x < width_ && y >= 0 && y < height_;
}

bool Model::isWin()
{
    for(int y{0}; y < height_; ++y)
        for(int x{0}; x < width_; ++x)
            if(field_[y][x].state == CellState::CLOSED)
                return false;
    return true;
}

void Model::notifyAllListeners()
{
    for(auto &l :listeners_)
        l->updateView();
}

