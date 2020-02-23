#include "MockTrainController.h"
#include <QDebug>

MockTrainController::MockTrainController()
{
    for (int i = 0; i < 26; i++) this->triggerCount[i] = 0;
}


void MockTrainController::setSpeed(int val)
{
}

void MockTrainController::setLight(int val)
{
}

void MockTrainController::setDirection(bool reverse)
{
}

int MockTrainController::getSpeed()
{
    return 0;
}

bool MockTrainController::getDirection()
{
    return false;
}

void MockTrainController::setRelay(uint8_t num, bool active)
{
}

void MockTrainController::triggerRelay(uint8_t num)
{
    qDebug() << "Trigger " << num;
    this->triggerCount[num]++;
}
