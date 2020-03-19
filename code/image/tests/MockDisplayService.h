#ifndef MOCKDISPLAYSERVICE_H
#define MOCKDISPLAYSERVICE_H

#include <IDisplayService.h>



class MockDisplayService : public IDisplayService
{
public:
    MockDisplayService();
    ~MockDisplayService();

    // IDisplayService interface
public:
    void setViewType(ViewType vt) override;
    void start() override;
    void init(QGraphicsView *view) override;
    void removeItem(QString name) override;
    QGraphicsItem *item(QString name);
    QGraphicsPolygonItem *polygonItem(QString name);
    void updateWayPoint() override;
    void setWaypoint(QPoint w, QPoint trackPoint) override;
    void setProbe(QVideoProbe *probe) override;
    void createLocomotiveItem(QString name, ViewType viewType) override;
    void createWagonItem(QString name, ViewType viewType) override;
    void createPixmapItem(QString name, ViewType viewType, QString fileName, bool selectable) override;
    void createAnnotationItem(QString name, ViewType viewType, QString fileName, int radius, bool selectable) override;

    QMap<QString,QGraphicsPolygonItem*> wagons;
    QGraphicsPolygonItem* loco;

    bool itemExists(QString name) override;
    void setTrackPen(QString name, QPen p) override;
    void setTrackPath(QString name, QPainterPath p) override;
    void createTrackItem(QString name, ViewType viewType) override;

    void setPosition(QString name, QPoint pos, DisplayPosition align) override;
    void setZValue(QString name, int val) override;
    void setPixmap(QString name, QPixmap) override;
    void setPolygon(QString name, QPolygon) override;
    void setBrush(QString name, QBrush) override;
};

#endif // MOCKDISPLAYSERVICE_H
