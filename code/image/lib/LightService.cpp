#include "LightService.h"

LightService::LightService(TrainController* trainController, int initialLevel)
{
    this->trainController = trainController;

    this->setLevel(initialLevel);
}

void LightService::setLevel(int level)
{
    this->level = level;
    this->trainController->setLight(level);
}

int LightService::getLevel()
{
    return this->level;
}
