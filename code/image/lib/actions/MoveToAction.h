#ifndef MOVETOACTION_H
#define MOVETOACTION_H

#include "Action.h"
#include "constants.h"
#include <QPoint>
#include <QString>


class MoveToAction : public Action
{
public:
    enum class StopPosition
    {
        Before,
        Within,
        After
    };

    MoveToAction(QPoint pos, QString currentTrack, QString targetTrack, bool reverse=false, int radius=DETECT_RECT, StopPosition where=StopPosition::Within);


    QString getTargetTrack();
    int getRadius();
    QPoint getPosition();
    StopPosition getWhere();
    bool getReverse();

    void onEnterTurnout(SplitterAnnotation* sa) override;
    void onTrainMoved(Train* train) override;

private:
    StopPosition where;
    int radius;
    QPoint pos;
    bool enteredZone;
    bool reverse;
    QString currentTrack;
    QString targetTrack;

protected:
    void start() override;

};

#endif // MOVETOACTION_H
