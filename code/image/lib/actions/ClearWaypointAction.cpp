#include "ClearWaypointAction.h"

ClearWaypointAction::ClearWaypointAction()
{

}


QString ClearWaypointAction::toString()
{
    QString str;
    QTextStream(&str) << "Clear Waypoint";

    return str;
}

void ClearWaypointAction::start(QVector<Annotation *> annotationsInRange)
{
    this->railroadLogicService->setWaypoint(QPoint());
}
