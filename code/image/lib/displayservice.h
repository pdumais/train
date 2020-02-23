#pragma once

#include "IDisplayService.h"

#include <QMediaPlayer>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QObject>
#include <QGraphicsVideoItem>
#include <QGraphicsItemGroup>
#include <QCamera>
#include <QVideoProbe>

class DisplayService: public IDisplayService
{
    Q_OBJECT
public:
    DisplayService();
    ~DisplayService() override;

    void setViewType(ViewType vt) override;
    void start() override;
    void init(QGraphicsView* view) override;

    void removeItem(QString name) override;
    QGraphicsPathItem* track(QString name) override;
    QGraphicsItem* item(QString name) override;
    QGraphicsPolygonItem* polygonItem(QString name) override;
    void updateWayPoint() override;
    void setWaypoint(QPoint w, QPoint trackPoint) override;
    void setProbe(QVideoProbe *probe) override;


    QGraphicsPolygonItem* createLocomotiveItem(QString name, ViewType viewType) override;
    QGraphicsPolygonItem* createWagonItem(QString name, ViewType viewType) override;
    QGraphicsPathItem* createTrackItem(QString name, ViewType viewType) override;
    QGraphicsPixmapItem* createPixmapItem(QString name, ViewType viewType, QString fileName, bool selectable=false) override;
    QGraphicsEllipseItem* createAnnotationItem(QString name, ViewType viewType, QString fileName, int radius, bool selectable=false) override;

    static QGraphicsPixmapItem* debugPixmap;
public slots:
    void on_operation_item_selected();


signals:
    void trainPositionChanged(QPoint);
    void dropDetected(QPoint);
    void annotationDropped(QString name, QPoint);
    void annotationSelected(QString name);
    void waypointSet(QPoint);


private:
    struct ItemInfo
    {
        QString name;
        QGraphicsItem* item;
        ViewType viewType;      // null if application for all views
    };

    QCamera *camera;
    QGraphicsScene* operationScene;
    QGraphicsScene* learningScene;
    QGraphicsView* view;
    QPoint wayPoint;
    QPoint wayPointEnd;

    // Overlay components
    QMap<QString, ItemInfo> items;
    QGraphicsVideoItem* videoItem;

    QGraphicsVideoItem* createVideoItem();

    QGraphicsItem* addGraphicsItem(QString name, QGraphicsItem* item, ViewType viewType, bool selectable=false);
};




