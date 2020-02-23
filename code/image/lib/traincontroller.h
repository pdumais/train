#ifndef TRAINCONTROLLER_H
#define TRAINCONTROLLER_H

#include "configuration.h"
#include "constants.h"
#include <QObject>
#include <QSerialPort>
#include <QTime>
#include <ITrainController.h>


class TrainController: public ITrainController
{
    Q_OBJECT
public:
    TrainController(Configuration *conf);
    ~TrainController() override;

    void setSpeed(int val) override;
    void setLight(int val) override;
    void setDirection(bool reverse) override;
    int getSpeed() override;
    bool getDirection() override;

    void setRelay(uint8_t num, bool active) override;
    void triggerRelay(uint8_t num) override;

private:
    struct RelayRecord
    {
        int index;
        bool abused;
        QTime times[RELAY_RECORD_SIZE];
    };

    bool direction;
    int speed;
    int light;
    bool relaysStates[26];
    QString port;
    QSerialPort* serialPort;

    RelayRecord relayActivationRecords[MAX_RELAY];
    bool checkRelayAbuse(uint8_t num);

};

#endif // TRAINCONTROLLER_H
