#include "MockDisplayService.h"

MockDisplayService::MockDisplayService()
{
    this->loco = new QGraphicsPolygonItem();
    for (int i =0 ; i <20; i++)
    {
        QString name;
        QTextStream(&name) << "wagon" << i;
        this->wagons[name] = new QGraphicsPolygonItem();
    }
}


MockDisplayService::~MockDisplayService()
{
    delete loco;
    for (auto w : this->wagons.values()) delete w;
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

QGraphicsItem *MockDisplayService::item(QString name)
{
    if (name == "locomotive") return this->loco;
    if (this->wagons.contains(name)) return this->wagons[name];

    return 0;
}

QGraphicsPolygonItem *MockDisplayService::polygonItem(QString name)
{
    if (name == "locomotive") return this->loco;
    if (this->wagons.contains(name)) return this->wagons[name];
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

void MockDisplayService::createLocomotiveItem(QString name, IDisplayService::ViewType viewType)
{
}

void MockDisplayService::createWagonItem(QString name, IDisplayService::ViewType viewType)
{
}

void MockDisplayService::createPixmapItem(QString name, IDisplayService::ViewType viewType, QString fileName, bool selectable)
{
}

void MockDisplayService::createAnnotationItem(QString name, IDisplayService::ViewType viewType, QString fileName, int radius, bool selectable)
{
}


bool MockDisplayService::itemExists(QString name)
{
    return false;
}


void MockDisplayService::setPosition(QString name, QPoint pos, DisplayPosition align)
{
}

void MockDisplayService::setZValue(QString name, int val)
{
}

void MockDisplayService::createTrackItem(QString name, ViewType viewType)
{
}

void MockDisplayService::setTrackPen(QString name, QPen p)
{
}

void MockDisplayService::setTrackPath(QString name, QPainterPath p)
{
}

void MockDisplayService::setPixmap(QString name, QPixmap p)
{
}

void MockDisplayService::setPolygon(QString name, QPolygon p)
{
    if (!this->wagons.contains(name)) return;
    auto g = dynamic_cast<QGraphicsPolygonItem*>(this->wagons[name]);
    if (!g) return;
    g->setPolygon(p);
}

void MockDisplayService::setBrush(QString name, QBrush b)
{
    if (!this->wagons.contains(name)) return;
    auto g = dynamic_cast<QAbstractGraphicsShapeItem*>(this->wagons[name]);
    if (!g) return;
    g->setBrush(b);
}
