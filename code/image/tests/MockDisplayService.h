#ifndef MOCKDISPLAYSERVICE_H
#define MOCKDISPLAYSERVICE_H

#include <IDisplayService.h>



class MockDisplayService : public IDisplayService
{
public:
    MockDisplayService();

    // IDisplayService interface
public:
    void setViewType(ViewType vt) override;
    void start() override;
    void init(QGraphicsView *view) override;
    void removeItem(QString name) override;
    QGraphicsPathItem *track(QString name) override;
    QGraphicsItem *item(QString name) override;
    QGraphicsPolygonItem *polygonItem(QString name) override;
    void updateWayPoint() override;
    void setWaypoint(QPoint w, QPoint trackPoint) override;
    void setProbe(QVideoProbe *probe) override;
    QGraphicsPolygonItem *createLocomotiveItem(QString name, ViewType viewType) override;
    QGraphicsPolygonItem *createWagonItem(QString name, ViewType viewType) override;
    QGraphicsPathItem *createTrackItem(QString name, ViewType viewType) override;
    QGraphicsPixmapItem *createPixmapItem(QString name, ViewType viewType, QString fileName, bool selectable) override;
    QGraphicsEllipseItem *createAnnotationItem(QString name, ViewType viewType, QString fileName, int radius, bool selectable) override;
};

#endif // MOCKDISPLAYSERVICE_H
