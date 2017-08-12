#include "view.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "model.h"
#include "controller.h"

View::View(Model *model, Controller *controller):
    pModel_(model), pController_(controller)
{
    initAllegro5();
    pMainWindow_ = al_create_display(WINDOW_WIDTH, WINDOW_HEIGHT);
    if(!pMainWindow_)
    {
        al_show_native_message_box(0, "Error", 0, "Could not create Allegro Window", 0, 0);
    }
    al_set_window_position(pMainWindow_, WINDOW_X, WINDOW_Y);
    al_set_window_title(pMainWindow_, TITLE_OF_PROGRAM.c_str());
    //Fonts
    initFonts();
    //Sounds
    initSounds();
    //Timers
    pTimer_ = al_create_timer(1.0f / FPS);
}

View::~View()
{
    al_destroy_font(pFontSmall_);
    al_destroy_font(pFontMiddle_);
    al_destroy_font(pFontLarge_);
    al_destroy_sample(pSoundVictory_);
    al_destroy_sample(pSoundDefeat_);
    al_destroy_sample(pSoundExplosion_);
    al_destroy_timer(pTimer_);
    al_destroy_display(pMainWindow_);
}

void View::run()
{
    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_display_event_source(pMainWindow_));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(pTimer_));
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
                for(int y{0}; y < pModel_->fieldHeight(); ++y)
                {
                    for(int x{0}; x < pModel_->fieldWidth(); ++x)
                    {
                        switch(pModel_->getCellState(x, y))
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
                                if(pModel_->isCellMined(x,y))
                                {
                                    bool isExploded = x == pModel_->getExplosionX() &&
                                            y == pModel_->getExplosionY();
                                    drawMine(x,y,isExploded);
                                }
                                else
                                {
                                    drawOpenedCell(x, y, pModel_->countMinesAround(x, y));
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
                    al_stop_timer(pTimer_);
                    al_resize_display(pMainWindow_, WINDOW_WIDTH, WINDOW_HEIGHT);
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
                if(pModel_->gameState() == Model::GameState::PLAY)
                {
                    if(events.mouse.button & 1)
                    {
                        pController_->leftClick(cursorX / CELL_WIDTH, cursorY / CELL_HEIGHT);
                        if(pModel_->gameState() == Model::GameState::VICTORY)
                        {
                            al_stop_timer(pTimer_);
                            if(pSoundVictory_ != NULL)
                                al_play_sample(pSoundVictory_, 1.0f, 0.0f, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                        }
                        else if(pModel_->gameState() == Model::GameState::DEFEAT)
                        {
                            al_stop_timer(pTimer_);
                            if(pSoundExplosion_ != NULL)
                                al_play_sample(pSoundExplosion_, 1.0f, 0.0f, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                            if(pSoundDefeat_ != NULL)
                                al_play_sample(pSoundDefeat_, 1.0f, 0.0f, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
                        }
                    }
                    else if(events.mouse.button & 2)
                    {
                        pController_->rightClick(cursorX / CELL_WIDTH, cursorY / CELL_HEIGHT);
                        if(pModel_->gameState() == Model::GameState::VICTORY)
                        {
                            al_stop_timer(pTimer_);
                            if(pSoundVictory_ != NULL)
                                al_play_sample(pSoundVictory_, 1.0f, 0.0f, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
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

void View::initAllegro5()
{
    if(!al_init())
    {
        al_show_native_message_box(0, "Error", 0, "Could not initialize Allegro 5", 0, 0);
        throw std::runtime_error("Could not initialize allegro5");
    }
    al_init_font_addon();
    if(!al_init_ttf_addon())
        throw std::runtime_error("Could not initialize font addon");
    if(!al_init_primitives_addon())
        throw std::runtime_error("Could not initialize primitives addon");
    if(!al_install_audio())
        throw std::runtime_error("Could not install audio");
    if(!al_init_acodec_addon())
        throw std::runtime_error("Could not initialize scodec addon");
    if(!al_install_keyboard())
        throw std::runtime_error("Could not install keyboard");
    if(!al_install_mouse())
        throw std::runtime_error("Could not install mouse");
}

void View::initFonts()
{
    pFontSmall_ = al_load_ttf_font("Resources/Fonts/orbitron-light.ttf", FONT_SMALL_SIZE, 0);
    pFontMiddle_ = al_load_ttf_font("Resources/Fonts/orbitron-light.ttf", FONT_MIDDLE_SIZE, 0);
    pFontLarge_ = al_load_ttf_font("Resources/Fonts/orbitron-light.ttf", FONT_LARGE_SIZE, 0);
}

void View::initSounds()
{
    al_reserve_samples(3);
    pSoundVictory_ =  al_load_sample("Resources/Sounds/Victory.wav");
    pSoundDefeat_ =  al_load_sample("Resources/Sounds/Defeat.wav");
    pSoundExplosion_ = al_load_sample("Resources/Sounds/Explosion.wav");
}

void View::startGameWithSelectedLevel()
{
    al_stop_timer(pTimer_);
    al_set_timer_count(pTimer_, 0);
    switch (_currentMenuItem) {
    case 0:
        pController_->changeLevel(1);
        break;
    case 1:
        pController_->changeLevel(2);
        break;
    case 2:
        pController_->changeLevel(3);
        break;
    case 3:
        _done = true;
        break;
    default:
        break;
    }
    //std::cout << "newWidth = " << _pModel->fieldWidth() << std::endl;
    //std::cout << "newHeight = " << _pModel->fieldHeight() << std::endl;
    al_resize_display(pMainWindow_, pModel_->fieldWidth() * CELL_WIDTH,
                      pModel_->fieldHeight() * CELL_HEIGHT + GAP_FOR_GAME_INFO);
    _isMenuActive = false;

    al_start_timer(pTimer_);
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
        al_draw_text(pFontMiddle_, color, numMinesAround == 1 ? tx + 4: tx, ty, 0,
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
    al_draw_text(pFontMiddle_, al_map_rgb(255,255,0), x * CELL_WIDTH + 7, y * CELL_HEIGHT + 7, 0, "?");
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
    if(pModel_->level() == 1)
        drawFlag(0, 9);
    else
        drawFlag(0, 16);
    std::string status;
    switch (pModel_->gameState()) {
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
    ss << pModel_->numFlags() << "/" << pModel_->numMines() << " STATUS:" << status;
    ALLEGRO_FONT *pFont = (pModel_->level() == 1) ? pFontSmall_ : pFontMiddle_;
    al_draw_text(pFont, al_map_rgb(0,255,0), 35, al_get_display_height(pMainWindow_) - 45,
                 0, ss.str().c_str());
}

void View::drawTime() const
{
    auto count_ms = al_get_timer_count(pTimer_) * 1000 / FPS;
    int hour = count_ms / 3600000;
    int min = (count_ms % 3600000) / 60000;
    int sec = ((count_ms % 3600000) % 60000) / 1000;
    int ms =  ((count_ms % 3600000) % 60000) % 1000;
    std::stringstream ss;
    ss << "Time: " << hour << ":" << min << ":" << sec << ":" << ms;
    al_draw_text(pFontSmall_, al_map_rgb(255,0,0), 35,
                 al_get_display_height(pMainWindow_) - 25,
                 0, ss.str().c_str());
}

void View::drawMenu() const
{
    al_draw_text(pFontLarge_, al_map_rgb(0,127,234), WINDOW_WIDTH / 8, WINDOW_HEIGHT / 8, 0, "MINESWEEPER");
    al_draw_text(pFontLarge_, al_map_rgb(255,0,0), WINDOW_WIDTH / 3, WINDOW_HEIGHT / 4, 0, "MENU");
    for(unsigned short int i{0}; i < _menuItems.size(); ++i)
    {
        ALLEGRO_COLOR color = i == _currentMenuItem ? al_map_rgb(255,255,0) : al_map_rgb(0, 148, 255);
        al_draw_text(pFontMiddle_, color, WINDOW_WIDTH / 5,
                     WINDOW_HEIGHT / 3 + (i + 1) * 40, 0, _menuItems[i].c_str());
    }
}
