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
    inline int level() const {return level_; }
    inline int fieldWidth() const { return width_; }
    inline int fieldHeight() const { return height_; }
    inline int numMines() const { return numMines_; }
    inline int numFlags() const { return numFlags_; }
    inline bool isCellMined(int x, int y) const { return field_[y][x].isMined; }
    inline CellState getCellState(int x, int y) const { return field_[y][x].state; }
    inline int getExplosionX() const { return explosionX_; }
    inline int getExplosionY() const { return explosionY_; }
    inline GameState gameState() const { return gameState_; }
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
    const int levelsData_[3][3] = {
        {9, 9, 10},       //Field size x, field size y and number of mines
        {16, 16, 40},
        {30, 16, 99}
    };
    struct Cell
    {
        bool isMined;
        CellState state;
    };
    int level_, width_, height_;
    int numMines_{0}, numFlags_{0};
    bool isFirstMove_ {true};
    int explosionX_{-1}, explosionY_{-1};
    Cell field_[FIELD_HEIGHT_MAX][FIELD_WIDTH_MAX];
    GameState gameState_ {GameState::PLAY};
    std::list<ModelListener*> listeners_;
    void createField();
    void mineField(int numMines);
    bool isCellCoordsInRange(int x, int y);
    bool isWin();
    void notifyAllListeners();
};

#endif // MODEL_H
