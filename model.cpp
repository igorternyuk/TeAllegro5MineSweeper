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
    if(_gameState == GameState::PLAY && isCellCoordsInRange(cx,cy) &&
       _field[cy][cx].state == CellState::CLOSED)
    {
        _field[cy][cx].state = CellState::OPENED;
        if(_field[cy][cx].isMined)
        {
            _gameState = GameState::DEFEAT;
            _explosionX = cx;
            _explosionY = cy;
            //Show all mines
            for(int y{0}; y < _height; ++y)
            {
                for(int x{0}; x < _width; ++x)
                {
                    if(_field[y][x].isMined && _field[y][x].state != CellState::FLAGGED)
                        _field[y][x].state = CellState::OPENED;
                }
            }
            notifyAllListeners();
            return;
        }
        if(isWin())
        {
            _gameState = GameState::VICTORY;
            notifyAllListeners();
            return;
        }
        if(_isFirstMove)
        {
            mineField(_numMines);
            _isFirstMove = false;
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
                    if(_field[neighbourY][neighbourX].state == CellState::CLOSED)
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
        switch(_field[y][x].state)
        {
            case CellState::CLOSED:
                if(_numFlags < _numMines)
                {
                    ++_numFlags;
                    _field[y][x].state = CellState::FLAGGED;
                }
                else
                {
                    _field[y][x].state = CellState::QUESTIONED;
                }
                break;
            case CellState::FLAGGED:
                --_numFlags;
                _field[y][x].state = CellState::QUESTIONED;
                break;
            case CellState::QUESTIONED:
                _field[y][x].state = CellState::CLOSED;
                break;
            case CellState::OPENED:
                break;
        }
        if(isWin())
        {
            _gameState = GameState::VICTORY;
        }
        notifyAllListeners();
    }
}

void Model::startNewGame()
{
    changeLevel(_level);
}

void Model::changeLevel(int level)
{
    //std::cout << "level = " << level << std::endl;
    if(level < 1 || level > 3)
        level = 1;
    _width = _levelsData[level - 1][0];
    _height = _levelsData[level - 1][1];
    _numMines = _levelsData[level - 1][2];
    _level = level;
    _numFlags = 0;
    _isFirstMove = true;
    _explosionX = -1;
    _explosionY = -1;
    _gameState = GameState::PLAY;
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
            if(_field[neighbourY][neighbourX].isMined)
                ++counter;
        }
    }
    return counter;
}

void Model::addListener(ModelListener *listener)
{
    _listeners.push_back(listener);
}

void Model::removeListener(ModelListener *listener)
{
    _listeners.remove(listener);
}

void Model::createField()
{
    for(int y{0}; y < _height; ++y)
    {
        for(int x{0}; x < _width; ++x)
        {
            _field[y][x].state = CellState::CLOSED;
            _field[y][x].isMined = false;
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
            randX = rand() % _width;
            randY = rand() % _height;
        }
        while(_field[randY][randX].isMined || _field[randY][randX].state == CellState::OPENED);
        _field[randY][randX].isMined = true;
    }
    notifyAllListeners();
}

bool Model::isCellCoordsInRange(int x, int y)
{
    return x >= 0 && x < _width && y >= 0 && y < _height;
}

bool Model::isWin()
{
    for(int y{0}; y < _height; ++y)
        for(int x{0}; x < _width; ++x)
            if(_field[y][x].state == CellState::CLOSED)
                return false;
    return true;
}

void Model::notifyAllListeners()
{
    for(auto &l :_listeners)
        l->updateView();
}

