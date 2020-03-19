#pragma once

#include <QMediaPlayer>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QObject>
#include <QGraphicsVideoItem>
#include <QGraphicsItemGroup>
#include <QCamera>
#include <QVideoProbe>
#include <memory>

class IDisplayService: public QObject
{
    Q_OBJECT
public:
    IDisplayService() {}
    virtual ~IDisplayService() {}

    enum class ViewType
    {
        All,
        Operation,
        Learning,
        Debug,
    };

    enum class GraphicsObjectType
    {
        Annotation,
        Locomotive,
        Track,
        Wagon,
        Waypoint
    };

    virtual void setViewType(ViewType vt)=0;
    virtual void start()=0;
    virtual void init(QGraphicsView* view)=0;
    virtual void removeItem(QString name)=0;

    virtual bool itemExists(QString name) =0;
    virtual void setTrackPen(QString name, QPen p) = 0;
    virtual void setTrackPath(QString name, QPainterPath p)=0;
    virtual QGraphicsItem* item(QString name)=0;
    virtual QGraphicsPolygonItem* polygonItem(QString name)=0;
    virtual void updateWayPoint()=0;
    virtual void setWaypoint(QPoint w, QPoint trackPoint)=0;
    virtual void setProbe(QVideoProbe *probe)=0;
    virtual QGraphicsPolygonItem* createLocomotiveItem(QString name, ViewType viewType)=0;
    virtual QGraphicsPolygonItem* createWagonItem(QString name, ViewType viewType)=0;
    virtual void createTrackItem(QString name, ViewType viewType)=0;
    virtual QGraphicsPixmapItem* createPixmapItem(QString name, ViewType viewType, QString fileName, bool selectable=false)=0;
    virtual QGraphicsEllipseItem* createAnnotationItem(QString name, ViewType viewType, QString fileName, int radius, bool selectable=false)=0;

signals:
    void trainPositionChanged(QPoint);
    void dropDetected(QPoint);
    void annotationDropped(QString name, QPoint);
    void annotationSelected(QString name);

};




