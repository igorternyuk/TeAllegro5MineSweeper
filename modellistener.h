#ifndef MODELLISTENER_H
#define MODELLISTENER_H


class ModelListener
{
public:
    explicit ModelListener() {}
    virtual ~ModelListener() {}
    virtual void updateView() = 0;
};

#endif // MODELLISTENER_H
