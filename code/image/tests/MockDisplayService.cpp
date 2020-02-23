#include "MockDisplayService.h"

MockDisplayService::MockDisplayService()
{

}


void MockDisplayService::setViewType(IDisplayService::ViewType vt)
{
}

void MockDisplayService::start()
{
}

void MockDisplayService::init(QGraphicsView *view)
{
}

void MockDisplayService::removeItem(QString name)
{
}

QGraphicsPathItem *MockDisplayService::track(QString name)
{
    return 0;
}

QGraphicsItem *MockDisplayService::item(QString name)
{
    return 0;
}

QGraphicsPolygonItem *MockDisplayService::polygonItem(QString name)
{
    return 0;
}

void MockDisplayService::updateWayPoint()
{
}

void MockDisplayService::setWaypoint(QPoint w, QPoint trackPoint)
{
}

void MockDisplayService::setProbe(QVideoProbe *probe)
{
}

QGraphicsPolygonItem *MockDisplayService::createLocomotiveItem(QString name, IDisplayService::ViewType viewType)
{
    return 0;
}

QGraphicsPolygonItem *MockDisplayService::createWagonItem(QString name, IDisplayService::ViewType viewType)
{
    return 0;
}

QGraphicsPathItem *MockDisplayService::createTrackItem(QString name, IDisplayService::ViewType viewType)
{
    return 0;
}

QGraphicsPixmapItem *MockDisplayService::createPixmapItem(QString name, IDisplayService::ViewType viewType, QString fileName, bool selectable)
{
    return 0;
}

QGraphicsEllipseItem *MockDisplayService::createAnnotationItem(QString name, IDisplayService::ViewType viewType, QString fileName, int radius, bool selectable)
{
    return 0;
}
