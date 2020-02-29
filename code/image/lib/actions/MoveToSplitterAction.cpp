#include "MoveToSplitterAction.h"
#include <QTimer>

MoveToSplitterAction::MoveToSplitterAction(SplitterAnnotation* sa, QString currentTrack, QString targetTrack, bool reverse/*=false*/)
{
    this->reverse = reverse;
    this->targetTrack = targetTrack;
    this->currentTrack = currentTrack;
    this->targetSplitter = sa;
}

void MoveToSplitterAction::start(QVector<Annotation*> annotationsInRange)
{
    this->backingOutOfSplitter = nullptr;

    // Check if we are already within a compliant annotation
    for (auto it : annotationsInRange)
    {
        if (it != this->targetSplitter) continue;
        if (!it->getInRange()) break;

        SplitterAnnotation *sa = dynamic_cast<SplitterAnnotation*>(it);
        bool commingFromT0 = (this->reverse == sa->getClockWise());

        // The train has already entered that splitter so we might have to back out
        // of it if the track was not activated like it should have.
            
        // if we're comming from t1 or t2, we're going to t0 so there is no need to activate the track
        // If the target track is already the one that needed activatation then just proceed
        if (!commingFromT0 || (sa->getActiveTrack() == this->targetTrack))
        {
            this->done();
            return;
        }

        this->reverse = !this->reverse;
        backingOutOfSplitter = sa;
        break;
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


    //if (sa->getTrack1() == this->targetTrack || sa->getTrack2() == this->targetTrack)
    if (sa == this->targetSplitter)
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

QString MoveToSplitterAction::toString()
{
    QString str;
    QTextStream(&str) << "MoveToSplitterAction: " << (reverse?"backwards to ":"Forward to") << " " << targetTrack << " from " << currentTrack << " using splitter " << targetSplitter->toString();

    return str;
}
