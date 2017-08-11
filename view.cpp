#include "view.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include "model.h"
#include "controller.h"

View::View(Model *model, Controller *controller):
    _pModel(model), _pController(controller)
{
    if(initAllegro5())
    {
        _pMainWindow = al_create_display(WINDOW_WIDTH, WINDOW_HEIGHT);
        if(!_pMainWindow)
        {
            al_show_native_message_box(0, "Error", 0, "Could not create Allegro Window", 0, 0);
        }
        al_set_window_position(_pMainWindow, WINDOW_X, WINDOW_Y);
        al_set_window_title(_pMainWindow, TITLE_OF_PROGRAM.c_str());
        //Fonts
        _pFontSmall = al_load_ttf_font("Resources/Fonts/orbitron-light.ttf", FONT_SMALL_SIZE, 0);
        _pFontMiddle = al_load_ttf_font("Resources/Fonts/orbitron-light.ttf", FONT_MIDDLE_SIZE, 0);
        _pFontLarge = al_load_ttf_font("Resources/Fonts/orbitron-light.ttf", FONT_LARGE_SIZE, 0);
        //Sounds
        al_reserve_samples(3);
        _pSoundVictory =  al_load_sample("Resources/Sounds/Victory.wav");
        _pSoundDefeat =  al_load_sample("Resources/Sounds/Defeat.wav");
        _pSoundExplosion = al_load_sample("Resources/Sounds/Explosion.wav");
        //Timers
        _pTimer = al_create_timer(1.0f / FPS);
    }
}

View::~View()
{
    al_destroy_font(_pFontSmall);
    al_destroy_font(_pFontMiddle);
    al_destroy_font(_pFontLarge);
    al_destroy_sample(_pSoundVictory);
    al_destroy_sample(_pSoundDefeat);
    al_destroy_sample(_pSoundExplosion);
    al_destroy_timer(_pTimer);
    al_destroy_display(_pMainWindow);
}

void View::run()
{
    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_display_event_source(_pMainWindow));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(_pTimer));
    //al_start_timer(_pTimer);
    //Main loop of the game
    while(!_done)
    {
        if(_isMenuActive)
        {
            al_clear_to_color(al_map_rgb(0,0,0));
            drawMenu();
            al_flip_display();
        }
        else
        {
            if(_draw)
            {
                al_clear_to_color(al_map_rgb(0,0,0));
                //All drawing stuff
                for(int y{0}; y < _pModel->fieldHeight(); ++y)
                {
                    for(int x{0}; x < _pModel->fieldWidth(); ++x)
                    {
                        switch(_pModel->getCellState(x, y))
                        {
                            case Model::CellState::CLOSED:
                                drawClosedCell(x, y);
                                break;
                            case Model::CellState::FLAGGED:
                                drawFlaggedCell(x, y);
                                break;
                            case Model::CellState::QUESTIONED:
                                drawQuestionedCell(x, y);
                                break;
                            case Model::CellState::OPENED:
                                if(_pModel->isCellMined(x,y))
                                {
                                    bool isExploded = x == _pModel->getExplosionX() &&
                                            y == _pModel->getExplosionY();
                                    drawMine(x,y,isExploded);
                                }
                                else
                                {
                                    drawOpenedCell(x, y, _pModel->countMinesAround(x, y));
                                }
                            break;
                        }
                    }
                }
                drawGameStatus();
                drawTime();
                al_flip_display();
                _draw = false;
            }
        }
        //Events handling
        ALLEGRO_EVENT events;
        al_wait_for_event(event_queue, &events);
        if(events.type == ALLEGRO_EVENT_KEY_UP)
        {
            switch(events.keyboard.keycode)
            {
                case ALLEGRO_KEY_ESCAPE:
                    _done = true;
                    break;
                case ALLEGRO_KEY_N:
                    startGameWithSelectedLevel();
                    break;
                case ALLEGRO_KEY_M:
                    //Show menu
                    _isMenuActive = true;
                    al_stop_timer(_pTimer);
                    al_resize_display(_pMainWindow, WINDOW_WIDTH, WINDOW_HEIGHT);
                    _currentMenuItem = 0;
                    break;
                case ALLEGRO_KEY_UP:
                    if(_isMenuActive)
                    {
                      if(--_currentMenuItem < 0)
                        _currentMenuItem = _menuItems.size() - 1;
                    }
                    break;
                case ALLEGRO_KEY_DOWN:
                    if(_isMenuActive)
                    {
                      if(++_currentMenuItem > int(_menuItems.size() - 1))
                        _currentMenuItem = 0;
                    }
                    break;
                case ALLEGRO_KEY_ENTER:
                    if(_isMenuActive)
                    {
                        startGameWithSelectedLevel();
                    }
                default:
                    break;
            }
        }
        else if(events.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            //break;
            _done = true;
        }
        else if(events.type == ALLEGRO_EVENT_MOUSE_AXES)
        {
            //Select menu item by mouse clicking
            if(_isMenuActive)
            {
                int x = events.mouse.x;
                int y = events.mouse.y;
                //std::cout << "x: " << x << std::endl;
                //std::cout << "y: " << y << std::endl;
                if(x >= 132 && x <= 578 &&
                   y >= 210 && y < 364)
                {
                    if(y <= 236)
                        _currentMenuItem = 0;
                    else if(y <= 280)
                        _currentMenuItem = 1;
                    else if(y <= 320)
                        _currentMenuItem = 2;
                    else
                        _currentMenuItem = 3;
                }
            }
        }
        else if(events.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
        {
            int cursorX = events.mouse.x;
            int cursorY = events.mouse.y;
            if(!_isMenuActive)
            {
                if(_pModel->gameState() == Model::GameState::PLAY)
                {
                    if(events.mouse.button & 1)
                    {
                        _pController->leftClick(cursorX / CELL_WIDTH, cursorY / CELL_HEIGHT);
                        if(_pModel->gameState() == Model::GameState::VICTORY)
                        {
                            al_stop_timer(_pTimer);
                            if(_pSoundVictory != NULL)
                                al_play_sample(_pSoundVictory, 1.0f, 0.0f, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                        }
                        else if(_pModel->gameState() == Model::GameState::DEFEAT)
                        {
                            al_stop_timer(_pTimer);
                            if(_pSoundExplosion != NULL)
                                al_play_sample(_pSoundExplosion, 1.0f, 0.0f, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                            if(_pSoundDefeat != NULL)
                                al_play_sample(_pSoundDefeat, 1.0f, 0.0f, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                        }
                    }
                    else if(events.mouse.button & 2)
                    {
                        _pController->rightClick(cursorX / CELL_WIDTH, cursorY / CELL_HEIGHT);
                        if(_pModel->gameState() == Model::GameState::VICTORY)
                        {
                            al_stop_timer(_pTimer);
                            if(_pSoundVictory != NULL)
                                al_play_sample(_pSoundVictory, 1.0f, 0.0f, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                        }
                    }
                }
            }
            else
            {
                startGameWithSelectedLevel();
            }

        }
        else if(events.type == ALLEGRO_EVENT_TIMER)
        {
            _draw = true;
        }
    }
}

void View::updateView()
{
    _draw = true;
}

bool View::initAllegro5()
{
    if(!al_init())
    {
        al_show_native_message_box(0, "Error", 0, "Could not initialize Allegro 5", 0, 0);
        return false;
    }
    al_init_font_addon();
    al_init_ttf_addon();
    al_init_primitives_addon();
    al_install_audio();
    al_init_acodec_addon();
    al_install_keyboard();
    al_install_mouse();
    return true;
}

void View::startGameWithSelectedLevel()
{
    al_stop_timer(_pTimer);
    al_set_timer_count(_pTimer, 0);
    switch (_currentMenuItem) {
    case 0:
        _pController->changeLevel(1);
        break;
    case 1:
        _pController->changeLevel(2);
        break;
    case 2:
        _pController->changeLevel(3);
        break;
    case 3:
        _done = true;
        break;
    default:
        break;
    }
    //std::cout << "newWidth = " << _pModel->fieldWidth() << std::endl;
    //std::cout << "newHeight = " << _pModel->fieldHeight() << std::endl;
    al_resize_display(_pMainWindow, _pModel->fieldWidth() * CELL_WIDTH,
                      _pModel->fieldHeight() * CELL_HEIGHT + GAP_FOR_GAME_INFO);
    _isMenuActive = false;

    al_start_timer(_pTimer);
    _draw = true;
}

void View::drawClosedCell(int x, int y) const
{
    al_draw_filled_rectangle(x * CELL_WIDTH, y * CELL_HEIGHT, (x + 1) * CELL_WIDTH,
                             (y + 1) * CELL_HEIGHT, al_map_rgb(5, 139, 221));
    drawOutline(x, y);
}

void View::drawOutline(int x, int y) const
{
    al_draw_line(x * CELL_WIDTH + 1, y * CELL_WIDTH + 1, (x + 1) * CELL_WIDTH - 1,  y * CELL_WIDTH + 1,
                 al_map_rgb(150,201,233), 2);
    al_draw_line(x * CELL_WIDTH + 1, y * CELL_WIDTH + 1, x * CELL_WIDTH + 1,  (y + 1) * CELL_WIDTH - 1,
                 al_map_rgb(150,201,233), 2);
    al_draw_line((x + 1) * CELL_WIDTH - 1, (y + 1) * CELL_WIDTH - 1, (x + 1) * CELL_WIDTH - 1,
                 y * CELL_WIDTH + 1, al_map_rgb(29,114,167), 2);
    al_draw_line((x + 1) * CELL_WIDTH - 1, (y + 1) * CELL_WIDTH - 1, x * CELL_WIDTH + 1,
                 (y + 1) * CELL_WIDTH - 1, al_map_rgb(29,114,167), 1);
}

void View::drawOpenedCell(int x, int y, int numMinesAround) const
{
    al_draw_filled_rectangle(x * CELL_WIDTH, y * CELL_HEIGHT, (x + 1) * CELL_WIDTH,
                             (y + 1) * CELL_HEIGHT, al_map_rgb(200, 200, 200));
    drawOutlineSunken(x,y);
    if(numMinesAround > 0)
    {
        ALLEGRO_COLOR color;
        switch(numMinesAround)
        {
            case 1:
                color = al_map_rgb(0, 0, 255);
                break;
            case 2:
                color = al_map_rgb(255, 0, 0);
                break;
            case 3:
                color = al_map_rgb(0, 127, 0);
                break;
            case 4:
                color = al_map_rgb(255, 255, 0);
                break;
            case 5:
                color = al_map_rgb(127, 0, 0);
                break;
            case 6:
                color = al_map_rgb(0, 204,127);
                break;
            case 7:
                color = al_map_rgb(206,45,152);
                break;
            case 8:
                color = al_map_rgb(201, 151, 27);
                break;
            default:
                color = al_map_rgb(0,0,0);
                break;
        }
        int tx = x * CELL_WIDTH + 7;
        int ty = y * CELL_HEIGHT + 7;
        al_draw_text(_pFontMiddle, color, numMinesAround == 1 ? tx + 4: tx, ty, 0,
                     std::to_string(numMinesAround).c_str());
    }
}

void View::drawOutlineSunken(int x, int y) const
{
    al_draw_line(x * CELL_WIDTH + 1, y * CELL_WIDTH + 1, (x + 1) * CELL_WIDTH - 1,  y * CELL_WIDTH + 1,
                 al_map_rgb(100,100,100), 2);
    al_draw_line(x * CELL_WIDTH + 1, y * CELL_WIDTH + 1, x * CELL_WIDTH + 1,  (y + 1) * CELL_WIDTH - 1,
                 al_map_rgb(100,100,100), 2);
    al_draw_line((x + 1) * CELL_WIDTH - 1, (y + 1) * CELL_WIDTH - 1, (x + 1) * CELL_WIDTH - 1,
                 y * CELL_WIDTH + 1, al_map_rgb(240,240,240), 2);
    al_draw_line((x + 1) * CELL_WIDTH - 1, (y + 1) * CELL_WIDTH - 1, x * CELL_WIDTH + 1,
                 (y + 1) * CELL_WIDTH - 1, al_map_rgb(240,240,240), 2);
}

void View::drawFlag(int x, int y) const
{
    al_draw_line(x * CELL_WIDTH + 7, y * CELL_HEIGHT + 4, x * CELL_WIDTH + 7, (y + 1) * CELL_HEIGHT - 4,
                 al_map_rgb(220, 220, 0), 2);
    al_draw_filled_triangle(x * CELL_WIDTH + 8, y * CELL_HEIGHT + 4, (x + 1) * CELL_WIDTH - 4,
                            (y + 0.6) * CELL_HEIGHT, x * CELL_WIDTH + 8, (y + 0.6) * CELL_HEIGHT,
                            al_map_rgb(255,0,0));
}

void View::drawFlaggedCell(int x, int y) const
{
    drawClosedCell(x, y);
    drawFlag(x,y);
}

void View::drawQuestionedCell(int x, int y) const
{
    drawClosedCell(x, y);
    al_draw_text(_pFontMiddle, al_map_rgb(255,255,0), x * CELL_WIDTH + 7, y * CELL_HEIGHT + 7, 0, "?");
}

void View::drawMine(int x, int y, bool isExploded) const
{
    //background
    al_draw_filled_rectangle(x * CELL_WIDTH, y * CELL_HEIGHT, (x + 1) * CELL_WIDTH,
                             (y + 1) * CELL_HEIGHT, al_map_rgb(200, 200, 200));
    drawOutlineSunken(x,y);
    //circle
    ALLEGRO_COLOR color = isExploded ? al_map_rgb(255,0,0) : al_map_rgb(0,0,0);
    float xc = (x + 0.5) * CELL_WIDTH;
    float yc = (y + 0.5) * CELL_WIDTH;
    //rays
    al_draw_filled_circle(xc, yc, 6, color);
    for(int i = 0; i < 8; ++i)
    {
        float angle = 2 * M_PI * i / 8;
        al_draw_line(xc, yc, xc + 9 * cos(angle), yc + 9 * sin(angle), color, 2);
    }
}

void View::drawGameStatus() const
{
    if(_pModel->level() == 1)
        drawFlag(0, 9);
    else
        drawFlag(0, 16);
    std::string status;
    switch (_pModel->gameState()) {
    case Model::GameState::PLAY:
        status = "PLAY";
        break;
    case Model::GameState::VICTORY:
        status = "YOU WON!!!";
        break;
    case Model::GameState::DEFEAT:
        status = "YOU LOST!";
        break;
    default:
        status = "";
        break;
    }
    std::stringstream ss;
    ss << _pModel->numFlags() << "/" << _pModel->numMines() << " STATUS:" << status;
    ALLEGRO_FONT *pFont = (_pModel->level() == 1) ? _pFontSmall : _pFontMiddle;
    al_draw_text(pFont, al_map_rgb(0,255,0), 35, al_get_display_height(_pMainWindow) - 45,
                 0, ss.str().c_str());
}

void View::drawTime() const
{
    auto count_ms = al_get_timer_count(_pTimer) * 1000 / FPS;
    int hour = count_ms / 3600000;
    int min = (count_ms % 3600000) / 60000;
    int sec = ((count_ms % 3600000) % 60000) / 1000;
    int ms =  ((count_ms % 3600000) % 60000) % 1000;
    std::stringstream ss;
    ss << "Time: " << hour << ":" << min << ":" << sec << ":" << ms;
    al_draw_text(_pFontSmall, al_map_rgb(255,0,0), 35,
                 al_get_display_height(_pMainWindow) - 25,
                 0, ss.str().c_str());
}

void View::drawMenu() const
{
    al_draw_text(_pFontLarge, al_map_rgb(0,127,234), WINDOW_WIDTH / 8, WINDOW_HEIGHT / 8, 0, "MINESWEEPER");
    al_draw_text(_pFontLarge, al_map_rgb(255,0,0), WINDOW_WIDTH / 3, WINDOW_HEIGHT / 4, 0, "MENU");
    for(unsigned short int i{0}; i < _menuItems.size(); ++i)
    {
        ALLEGRO_COLOR color = i == _currentMenuItem ? al_map_rgb(255,255,0) : al_map_rgb(0, 148, 255);
        al_draw_text(_pFontMiddle, color, WINDOW_WIDTH / 5,
                     WINDOW_HEIGHT / 3 + (i + 1) * 40, 0, _menuItems[i].c_str());
    }
}
