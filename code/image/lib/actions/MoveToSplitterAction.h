#pragma once
#include "Action.h"
#include "constants.h"
#include <QPoint>
#include <QString>


class MoveToSplitterAction : public Action
{
public:
    MoveToSplitterAction(QString currentTrack, QString targetTrack, bool reverse=false);


    QString getTargetTrack();
    QString getCurrentTrack();
    bool getReverse();

    void onEnterTurnout(SplitterAnnotation* sa) override;
    void onLeaveTurnout(SplitterAnnotation* sa) override;

private:
    bool reverse;
    QString currentTrack;
    QString targetTrack;
    SplitterAnnotation* backingOutOfSplitter;

protected:
    void start(QVector<Annotation*> annotationsInRange) override;

};

