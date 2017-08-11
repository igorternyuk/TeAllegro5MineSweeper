#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <list>

class ModelListener;

class Model
{
public:
    enum class CellState{CLOSED, OPENED, FLAGGED, QUESTIONED};
    enum class GameState{PLAY, VICTORY, DEFEAT};
    explicit Model(int level = 2);
    inline int level() const {return _level; }
    inline int fieldWidth() const { return _width; }
    inline int fieldHeight() const { return _height; }
    inline int numMines() const { return _numMines; }
    inline int numFlags() const { return _numFlags; }
    inline bool isCellMined(int x, int y) const { return _field[y][x].isMined; }
    inline CellState getCellState(int x, int y) const { return _field[y][x].state; }
    inline int getExplosionX() const { return _explosionX; }
    inline int getExplosionY() const { return _explosionY; }
    inline GameState gameState() const { return _gameState; }
    void openCell(int x, int y);
    void markCell(int x, int y);
    void startNewGame();
    void changeLevel(int level);
    int countMinesAround(int x, int y);
    void addListener(ModelListener *listener);
    void removeListener(ModelListener *listener);
private:
    enum {
        FIELD_WIDTH_MAX = 30,
        FIELD_HEIGHT_MAX = 16
    };
    const int _levelsData[3][3] = {
        {9, 9, 10},       //Field size x, field size y and number of mines
        {16, 16, 40},
        {30, 16, 99}
    };
    struct Cell
    {
        bool isMined;
        CellState state;
    };
    int _level, _width, _height;
    int _numMines{0}, _numFlags {0};
    bool _isFirstMove {true};
    int _explosionX{-1}, _explosionY{-1};
    Cell _field[FIELD_HEIGHT_MAX][FIELD_WIDTH_MAX];
    GameState _gameState {GameState::PLAY};
    std::list<ModelListener*> _listeners;
    void createField();
    void mineField(int numMines);
    bool isCellCoordsInRange(int x, int y);
    bool isWin();
    void notifyAllListeners();
};

#endif // MODEL_H
