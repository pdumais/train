#ifndef MOCKTRAINCONTROLLER_H
#define MOCKTRAINCONTROLLER_H

#include <ITrainController.h>



class MockTrainController : public ITrainController
{
public:
    MockTrainController();

    // ITrainController interface
public:
    void setSpeed(int val) override;
    void setLight(int val) override;
    void setDirection(bool reverse) override;
    int getSpeed() override;
    bool getDirection() override;
    void setRelay(uint8_t num, bool active) override;
    void triggerRelay(uint8_t num) override;

    int  triggerCount[26];
};

#endif // MOCKTRAINCONTROLLER_H
