#pragma once

#include "traincontroller.h"

class LightService
{
public:
    LightService(TrainController* trainController, int initialLevel);
    void setLevel(int level);
    int getLevel();

private:
    TrainController* trainController;
    int level;
};

