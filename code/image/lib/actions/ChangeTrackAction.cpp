#include "ChangeTrackAction.h"

ChangeTrackAction::ChangeTrackAction(SplitterAnnotation *sa, QString source, QString target, bool commingFromT0)
{
    this->sa = sa;
    this->target = target;
    this->commingFromT0 = commingFromT0;
    this->source = source;

    this->reverseToGoThrough = (this->sa->getClockWise() == commingFromT0);

}

SplitterAnnotation* ChangeTrackAction::getSplitterAnnotation()
{
    return this->sa;
}

void ChangeTrackAction::start(QVector<Annotation*> annotationsInRange)
{
    this->currentState = ChangeTrackAction::State::Stage1;
    if (this->commingFromT0 || this->target == this->sa->getInputTrack())
    {
        this->nextState = ChangeTrackAction::State::End;
    }
    else
    {
        // After stage1, we'll need to cross the splitter again in the opposite direction
        // because the target is reachable in the opposite direction
        this->nextState = ChangeTrackAction::State::Stage2;
    }

    this->doingSecondPass = false;
    if (this->commingFromT0)
    {
        if (sa->getTrack1() == this->target)
        {
            this->railroadLogicService->activateSplitter(sa,false);
        }
        else if (sa->getTrack2() == this->target)
        {
            this->railroadLogicService->activateSplitter(sa,true);
        }
    }
    else
    {
        // We need to reach t0. So activate the track we're on.
        this->railroadLogicService->activateSplitter(sa,source==sa->getTrack2());
    }

    if (this->reverseToGoThrough)
    {
        this->railroadLogicService->startTrainReverse();
    }
    else
    {
        this->railroadLogicService->startTrainForward();
    }

    // We should already be inside the turnout at this point so we will exit now.
}

bool ChangeTrackAction::getCommingFromT0()
{
    return this->commingFromT0;
}

QString ChangeTrackAction::getTargetTrack()
{
    return this->target;
}

void ChangeTrackAction::onEnterTurnout(SplitterAnnotation* sa)
{
    qDebug() << "ChangeTrackAction::onEnterTurnout";


    if (sa->getTrack1() == this->target)
    {
        this->railroadLogicService->activateSplitter(sa,false);
    }
    else if (sa->getTrack2() == this->target)
    {
        this->railroadLogicService->activateSplitter(sa,true);
    }
    else
    {
        qDebug() << "We've met a turnout that is not on our track";
    }

}

void ChangeTrackAction::onLeaveTurnout(SplitterAnnotation* sa)
{
    if (sa != this->sa) return;

    qDebug() << "ChangeTrackAction::onLeaveTurnout(): commingFromT0=" << this->commingFromT0;
    this->railroadLogicService->stopTrain();

    if (this->nextState == ChangeTrackAction::State::Stage2)
    {
        this->onLeaveTurnoutStage2(sa);
    }
    else if (this->nextState == ChangeTrackAction::State::End)
    {
        this->onLeaveTurnoutEnd(sa);
    }


}

void ChangeTrackAction::onLeaveTurnoutStage2(SplitterAnnotation* sa)
{
    this->currentState = ChangeTrackAction::State::Stage2;
    this->nextState = ChangeTrackAction::State::End;

    // We're on T0 at the moment, so activate the target
    if (sa->getTrack1() == this->target)
    {
        this->railroadLogicService->activateSplitter(sa,false);
    }
    else if (sa->getTrack2() == this->target)
    {
        this->railroadLogicService->activateSplitter(sa,true);
    }


    // Now start the train in the opposite direction
    if (this->reverseToGoThrough)
    {
        // we sent backwards in the first pass, now go forward
        this->railroadLogicService->startTrainForward();
    }
    else
    {
        // we sent forward in the first pass, now go backwards
        this->railroadLogicService->startTrainReverse();
    }
}

void ChangeTrackAction::onLeaveTurnoutEnd(SplitterAnnotation* sa)
{
    this->done();
}

QString ChangeTrackAction::toString()
{
    QString str;
    QTextStream(&str) << "ChangeTrackAction: " << sa->toString() << " comming from t0=" << commingFromT0 << " from " << source << " to " << target;

    return str;
}
