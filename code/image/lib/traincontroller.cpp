#include "traincontroller.h"

#include <QTextStream>
#include <QDebug>
#include <unistd.h>

TrainController::TrainController(Configuration* conf)
{
    this->speed = 0;
    this->direction = false;
    this->port = conf->getControllerPort();

    this->serialPort = new QSerialPort();
    this->serialPort->setPortName(this->port);
    this->serialPort->open(QIODevice::ReadWrite);
    this->serialPort->setBaudRate(57600);

    for (int i = 0; i < MAX_RELAY; i++)
    {
        relayActivationRecords[i].index = 0;
        relayActivationRecords[i].abused = false;
    }
    for (int i =0 ; i < 26; i++) this->relaysStates[i] = false;
}

TrainController::~TrainController()
{
    QString str;
    QTextStream(&str) << "a0\nb0\nc0\nd0\ne0\nf0\ng0\nh0\ni0\nj0\nk0\nl0\nm0\nn0\no0\np0\nq0\n";
    this->serialPort->write(str.toStdString().c_str());
    this->serialPort->flush();
    delete this->serialPort;
}


void TrainController::setSpeed(int val)
{
    QString str;
    if (this->direction) // true if reverse
    {
        QTextStream(&str) << "n0\no" << val << "\n";
    }
    else
    {
        QTextStream(&str) << "o0\nn" << val << "\n";
    }
    this->serialPort->write(str.toStdString().c_str());
    this->speed = val;
    emit speedChanged(val);
}

void TrainController::setLight(int val)
{
    QString str;
    QTextStream(&str) << "m" << val << "\n";
    this->serialPort->write(str.toStdString().c_str());
    this->light = val;
}

void TrainController::setDirection(bool reverse)
{
    this->direction = reverse;
    if (this->speed == 0) return;

    int previousSpeed = this->speed;
    this->setSpeed(0);
    usleep(250000);
    this->setSpeed(previousSpeed);
}

int TrainController::getSpeed()
{
    return this->speed;
}

bool TrainController::getDirection()
{
    return this->direction;
}

void TrainController::setRelay(uint8_t num, bool active)
{
    if (num >=MAX_RELAY) return;
    if (this->relaysStates[num] == active) return;

    if (active && this->checkRelayAbuse(num)) return;
    if (active && (num < (FIRST_CROSSROAD_RELAY-'a')))
    {
        // Extra protection
        qDebug() << "ERROR: Relays below #" << FIRST_CROSSROAD_RELAY << "Should only be triggered";
        return;
    }

    char r = 'a' + num-1;

    QString str;
    QTextStream(&str) << r << (active?1:0) << "\n";
    this->serialPort->write(str.toStdString().c_str());

    this->relaysStates[num] = active;
}


void TrainController::triggerRelay(uint8_t num)
{
    char r = 'a' + num-1;

    if (this->checkRelayAbuse(num)) return;

    QString str;
    QTextStream(&str) << r << "2\n";
    this->serialPort->write(str.toStdString().c_str());

}


bool TrainController::checkRelayAbuse(uint8_t num)
{
    // Once a relay has been abused, we ban it. We force the user to restart the app as a precaution
    if (this->relayActivationRecords[num].abused) return true;

    this->relayActivationRecords[num].index++;
    if (this->relayActivationRecords[num].index >= RELAY_RECORD_SIZE) this->relayActivationRecords[num].index = 0;

    QTime current = QTime::currentTime();

    bool abused = true;
    for (int i = 0; i < RELAY_RECORD_SIZE; i++)
    {
        if (this->relayActivationRecords[num].times[i].isNull())
        {
            // There is at least one time that is was never initialized, so no abuse here.
            abused = false;
            break;
        }
        if (this->relayActivationRecords[num].times[i].secsTo(current) >= RELAY_MAX_TIME_ABUSE)
        {
            // There is at least one time that is greater than the threshold, so there is no abuse
            abused = false;
            break;
        }
    }

    if (abused)
    {
        qDebug() << "Disactivating relay " << num << " because it was abused";
        this->relayActivationRecords[num].abused = true;
        return true;
    }

    int index = this->relayActivationRecords[num].index;
    this->relayActivationRecords[num].times[index] = current;

    return false;
}
