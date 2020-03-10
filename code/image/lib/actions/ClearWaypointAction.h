#ifndef CLEARWAYPOINTACTION_H
#define CLEARWAYPOINTACTION_H

#include "Action.h"



class ClearWaypointAction : public Action
{
public:
    ClearWaypointAction();

    // Action interface
public:
    QString toString() override;

protected:
    void start(QVector<Annotation *> annotationsInRange) override;
};

#endif // CLEARWAYPOINTACTION_H
