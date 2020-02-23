#include "InitSplittersAction.h"
#include <QTimer>
#include <QEventLoop>
#include "Functions.h"

InitSplittersAction::InitSplittersAction()
{
}

void InitSplittersAction::start(QVector<Annotation*> annotationsInRange)
{
    for (auto ti : this->tracks)
    {
        if (ti.sa->getTrack2() == ti.track)
        {
            this->railroadLogicService->activateSplitter(ti.sa,true);
        }
        else
        {
            this->railroadLogicService->activateSplitter(ti.sa,false);
        }

        Functions::nonBlockingSleep(300);

    }
    this->done();
}

void InitSplittersAction::addTrackInfo(SplitterAnnotation* sa, QString track)
{
    TrackInfo ti;
    ti.sa = sa;
    ti.track = track;
    this->tracks.append(ti);
}

QString InitSplittersAction::toString()
{
    QString str;
    QTextStream(&str) << "InitSplittersAction: "; 

    return str;
}
