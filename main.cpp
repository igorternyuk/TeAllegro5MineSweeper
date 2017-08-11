#include <iostream>
#include "view.h"
#include "model.h"
#include "controller.h"

int main()
{
    Model model(2);
    Controller controller(&model);
    View view(&model, &controller);
    model.addListener(&view);
    view.run();
    return 0;
}
