#pragma once
#include "Action.h"

#include <splitterannotation.h>



class ChangeTrackAction : public Action
{
public:
    ChangeTrackAction(SplitterAnnotation *sa, QString source, QString target, bool commingFromT0);

    SplitterAnnotation* getSplitterAnnotation();
    bool getCommingFromT0();
    QString getTargetTrack();

   void onEnterTurnout(SplitterAnnotation* sa) override;
   void onLeaveTurnout(SplitterAnnotation* sa) override;

   QString toString() override;
protected:
   void start(QVector<Annotation*> annotationsInRange) override;

private:
    enum State
    {
        Stage1,
        Stage2,
        ExitOverlapping,
        End
    };

    SplitterAnnotation *sa;
    bool commingFromT0;
    QString target;
    QString source;
    State currentState;
    State nextState;

    void onLeaveTurnoutStage2(SplitterAnnotation* sa);
    void onLeaveTurnoutEnd(SplitterAnnotation* sa);

    bool doingSecondPass;
    bool reverseToGoThrough;
};


