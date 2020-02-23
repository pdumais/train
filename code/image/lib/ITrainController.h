#ifndef ITRAINCONTROLLER_H
#define ITRAINCONTROLLER_H

#include "configuration.h"
#include "constants.h"
#include <QObject>
#include <QSerialPort>
#include <QTime>


class ITrainController: public QObject
{
    Q_OBJECT
public:
    ITrainController() {}
    virtual ~ITrainController() {}

    virtual void setSpeed(int val)=0;
    virtual void setLight(int val)=0;
    virtual void setDirection(bool reverse)=0;
    virtual int getSpeed()=0;
    virtual bool getDirection()=0;

    virtual void setRelay(uint8_t num, bool active)=0;
    virtual void triggerRelay(uint8_t num)=0;


signals:
    void speedChanged(int val);


};

#endif // ITRAINCONTROLLER_H
