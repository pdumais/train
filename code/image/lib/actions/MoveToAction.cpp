#include "MoveToAction.h"
#include <QTimer>

MoveToAction::MoveToAction(QPoint pos, QString currentTrack, QString targetTrack, bool reverse/*=false*/, int radius/*=DETECT_RECT*/, StopPosition where/*=StopPosition::Within*/)
{
    this->where = where;
    this->radius = radius;
    this->pos = pos;
    this->reverse = reverse;
    this->targetTrack = targetTrack;
    this->currentTrack = currentTrack;
    this->enteredZone = false;
}

void MoveToAction::start()
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

QString MoveToAction::getTargetTrack()
{
    return this->targetTrack;
}

int MoveToAction::getRadius()
{
    return this->radius;
}

QPoint MoveToAction::getPosition()
{
    return this->pos;
}

MoveToAction::StopPosition MoveToAction::getWhere()
{
    return this->where;
}

bool MoveToAction::getReverse()
{
    return this->reverse;
}

void MoveToAction::onEnterTurnout(SplitterAnnotation* sa)
{
    qDebug() << "MoveToAction::onEnterTurnout";

    bool exitingSplitter = (sa->getClockWise() && !this->reverse) || (!sa->getClockWise() && this->reverse);

    // If we're exiting, it means that our target is sa->inputTracl
    if (exitingSplitter)
    {
        if (sa->getTrack1() == this->currentTrack)
        {
            qDebug("we're exiting through T1. Deactivate fork");
            this->railroadLogicService->activateSplitter(sa,false);
        }
        else if (sa->getTrack2() == this->currentTrack)
        {
            qDebug("we're exiting through T2. Activate fork");
            this->railroadLogicService->activateSplitter(sa,true);
        }
    }
    else
    {
        if (sa->getTrack1() == this->targetTrack)
        {
            qDebug("we're entering, gping to T1. Deactivate fork");
            this->railroadLogicService->activateSplitter(sa,false);
        }
        else if (sa->getTrack2() == this->targetTrack)
        {
            qDebug("we're entering, gping to T2. Activate fork");
            this->railroadLogicService->activateSplitter(sa,true);
        }
        else if (sa->getTrack1() == sa->getInputTrack())
        {
            qDebug("we're continuing on current track, gping to T1. Deactivate fork");
            this->railroadLogicService->activateSplitter(sa,false);
        }
        else if (sa->getTrack2() == sa->getInputTrack())
        {
            qDebug("we're continuing on current track, gping to T2. Activate fork");
            this->railroadLogicService->activateSplitter(sa,false);
        }
    }

}

void MoveToAction::onTrainMoved(Train* train)
{
   // QRegion tr(train);
    //QRegion target(this->pos.x()-radius,this->pos.y()-radius,radius*2,radius*2,  QRegion::RegionType::Ellipse);
    if (train->inRange(this->pos, this->radius))
    {
        this->enteredZone = true;
        if (this->where != MoveToAction::StopPosition::After)
        {
            this->railroadLogicService->stopTrain();
            this->done();
            return;
        }
    }
    else
    {
        if (this->enteredZone) // we've just existed
        {
            this->railroadLogicService->stopTrain();
            this->done();
            return;
        }
    }
}
