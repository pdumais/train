#include "MoveToSplitterAction.h"
#include <QTimer>

MoveToSplitterAction::MoveToSplitterAction(QString currentTrack, QString targetTrack, bool reverse/*=false*/)
{
    this->reverse = reverse;
    this->targetTrack = targetTrack;
    this->currentTrack = currentTrack;
}

void MoveToSplitterAction::start(QVector<Annotation*> annotationsInRange)
{
    this->backingOutOfSplitter = nullptr;
    // Check if we are already within a compliant annotation
    for (auto it : annotationsInRange)
    {
        SplitterAnnotation *sa = dynamic_cast<SplitterAnnotation*>(it);
        if (!sa) continue;
        if (!sa->getInRange()) continue;

        if (sa->getTrack1() == this->targetTrack || sa->getTrack2() == this->targetTrack)
        {
            // We are already in range. We should go in the opposite direction that was requested
            // until we are out
            this->reverse = !this->reverse;
            backingOutOfSplitter = sa;
            break;
        }
    }

    if (this->reverse)
    {
        this->railroadLogicService->startTrainReverse();
    }
    else
    {
        this->railroadLogicService->startTrainForward();
    }
}

QString MoveToSplitterAction::getTargetTrack()
{
    return this->targetTrack;
}

QString MoveToSplitterAction::getCurrentTrack()
{
    return this->currentTrack;
}

bool MoveToSplitterAction::getReverse()
{
    return this->reverse;
}

void MoveToSplitterAction::onEnterTurnout(SplitterAnnotation* sa)
{
    qDebug() << "MoveToSplitterAction::onEnterTurnout";


    if (sa->getTrack1() == this->targetTrack || sa->getTrack2() == this->targetTrack)
    {
        // We've reached the turnout that can lead us to the target track
        this->railroadLogicService->stopTrain();
        done();
        return;
    }

    // If we make it this far, it means we are seeing a turnout that can take us to a track
    // that we're not interested it. So take whatever path is needed to keep on going on the same track.
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

void MoveToSplitterAction::onLeaveTurnout(SplitterAnnotation* sa)
{
    if (sa == this->backingOutOfSplitter)
    {
        this->backingOutOfSplitter = nullptr;
        // Now move a bit back in the opposite direction to re-enter
        if (this->reverse)
        {
            this->railroadLogicService->startTrainForward();
        }
        else
        {
            this->railroadLogicService->startTrainReverse();
        }
    }
}
