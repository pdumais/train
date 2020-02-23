#ifndef MOVETOACTION_H
#define MOVETOACTION_H

#include "Action.h"
#include "constants.h"
#include <QPoint>
#include <QString>


class MoveToAction : public Action
{
public:
    MoveToAction(QString currentTrack, QPoint pos, int radius, bool reverse=false);


    QPoint getTargetPosition();
    bool getReverse();
    QString getCurrentTrack();

    void onEnterTurnout(SplitterAnnotation* sa) override;
    void onTrainMoved(Train* train) override;

   QString toString() override;
private:
    bool reverse;
    QString currentTrack;
    QPoint pos;
    int radius;

protected:
    void start(QVector<Annotation*> annotationsInRange) override;

};

#endif // MOVETOACTION_H
