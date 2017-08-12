#ifndef CONTROLLER_H
#define CONTROLLER_H

class Model;

class Controller
{
public:
    explicit Controller(Model *model);
    void leftClick(int x, int y);
    void rightClick(int x, int y);
    void newGame();
    void changeLevel(int level);
private:
    Model *pModel_;
};

#endif // CONTROLLER_H
