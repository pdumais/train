#pragma once

#include "Action.h"
#include <QString>
#include <QVector>
#include "splitterannotation.h"



class InitSplittersAction : public Action
{
public:
    InitSplittersAction();

    void addTrackInfo(SplitterAnnotation* sa, QString track);

    QString toString() override;
private:
    struct TrackInfo
    {
        SplitterAnnotation* sa;
        QString track;
    };

    QVector<TrackInfo> tracks;

protected:
    void start(QVector<Annotation*> annotationsInRange) override;

};

