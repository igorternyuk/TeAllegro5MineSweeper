#ifndef VIEW_H
#define VIEW_H
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <vector>
#include <string>
#include "modellistener.h"

////Last edit: 12.08.2017
/// @author Igor Ternyuk

class Model;
class Controller;

class View: public ModelListener
{
public:
    explicit View(Model *model, Controller *controller);
    ~View();
    void run();
    virtual void updateView();
private:
    const std::string TITLE_OF_PROGRAM = "TeMineSweeper";
    enum {
        WINDOW_WIDTH = 640,
        WINDOW_HEIGHT = 510,
        WINDOW_X = 200,
        WINDOW_Y = 100,
        FONT_SMALL_SIZE = 18,
        FONT_MIDDLE_SIZE = 24,
        FONT_LARGE_SIZE = 60,
        CELL_WIDTH = 30,
        CELL_HEIGHT = 30,
        GAP_FOR_GAME_INFO = 50,
        FPS = 20
    };
    bool _isMenuActive{true};
    short int _currentMenuItem {0};
    const std::vector<std::string> _menuItems = {
        "1.Beginner (9 X 9) - 10 mines",
        "2.Intermediate (16 X 16) - 40 mines",
        "3.Advanced (16 X 30) - 100 mines",
        "4.Quit game"
    };
    bool _draw{true};
    bool _done{false};
    Model *pModel_;
    Controller *pController_;
    ALLEGRO_DISPLAY *pMainWindow_;
    ALLEGRO_FONT *pFontSmall_, *pFontMiddle_, *pFontLarge_;
    ALLEGRO_SAMPLE *pSoundVictory_;
    //ALLEGRO_SAMPLE_INSTANCE *_pSampleInstanceVictory;
    ALLEGRO_SAMPLE *pSoundDefeat_;
    //ALLEGRO_SAMPLE_INSTANCE *_pSampleInstanceDefeat;
    ALLEGRO_SAMPLE *pSoundExplosion_;
    ALLEGRO_TIMER *pTimer_;
    void initAllegro5();
    void initFonts();
    void initSounds();
    void startGameWithSelectedLevel();
    void drawClosedCell(int x, int y) const;
    void drawOutline(int x, int y) const;
    void drawOpenedCell(int x, int y, int numMinesAround) const;
    void drawOutlineSunken(int x, int y) const;
    void drawFlag(int x, int y) const;
    void drawFlaggedCell(int x, int y) const;
    void drawQuestionedCell(int x, int y) const;
    void drawMine(int x, int y, bool isExploded) const;
    void drawGameStatus() const;
    void drawTime() const;
    void drawMenu() const;
};

#endif // VIEW_H
