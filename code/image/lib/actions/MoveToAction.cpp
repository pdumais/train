#include "MoveToAction.h"
#include <QTimer>

MoveToAction::MoveToAction(QString currentTrack, QPoint pos, int radius,  bool reverse/*=false*/)
{
    this->reverse = reverse;
    this->pos = pos;
    this->currentTrack = currentTrack;
    this->radius = radius;
}

void MoveToAction::start(QVector<Annotation*> annotationsInRange)
{
    if (this->reverse)
    {
        this->railroadLogicService->startTrainReverse();
    }
    else
    {
        this->railroadLogicService->startTrainForward();
    }
}

QPoint MoveToAction::getTargetPosition()
{
    return this->pos;
}

QString MoveToAction::getCurrentTrack()
{
    return this->currentTrack;
}

void MoveToAction::onEnterTurnout(SplitterAnnotation* sa)
{
    qDebug() << "MoveToSplitterAction::onEnterTurnout";

    if (sa->getTrack1() == this->currentTrack)
    {
        this->railroadLogicService->activateSplitter(sa,false);
    }
    else if (sa->getTrack2() == this->currentTrack)
    {
        this->railroadLogicService->activateSplitter(sa,true);
    }
    else
    {
        qDebug() << "We've met a turnout that is not on our track";
    }
}

bool MoveToAction::getReverse()
{
    return this->reverse;
}

void MoveToAction::onTrainMoved(Train* train)
{
    if (train->inRange(this->pos, this->radius))
    {
        this->railroadLogicService->stopTrain();
        this->done();
    }
}
