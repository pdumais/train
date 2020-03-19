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
    void setTrackPen(QString name, QPen p) override;
    void setTrackPath(QString name, QPainterPath p) override;
    void updateWayPoint() override;
    void setWaypoint(QPoint w, QPoint trackPoint) override;
    void setProbe(QVideoProbe *probe) override;
    bool itemExists(QString name) override;


    void createLocomotiveItem(QString name, ViewType viewType) override;
    void createWagonItem(QString name, ViewType viewType) override;
    void createTrackItem(QString name, ViewType viewType) override;
    void createPixmapItem(QString name, ViewType viewType, QString fileName="", bool selectable=false) override;
    void createAnnotationItem(QString name, ViewType viewType, QString fileName, int radius, bool selectable=false) override;
    void setPosition(QString name, QPoint pos, DisplayPosition align=DisplayPosition::TopLeft) override;
    void setZValue(QString name, int val) override;
    void setPixmap(QString name, QPixmap) override;
    void setPolygon(QString name, QPolygon) override;
    void setBrush(QString name, QBrush) override;

public slots:
    void on_operation_item_selected();
    void on_learning_track_updated(QPainterPath p);

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
    QGraphicsScene* debugScene;
    QGraphicsView* view;
    QPoint wayPoint;
    QPoint wayPointEnd;

    // Overlay components
    QMap<QString, ItemInfo> items;
    QGraphicsVideoItem* videoItem;

    QGraphicsVideoItem* createVideoItem();

    QGraphicsItem* addGraphicsItem(QString name, QGraphicsItem* item, ViewType viewType, bool selectable=false);
    QGraphicsPolygonItem* polygonItem(QString name);
    QGraphicsItem* item(QString name);
};




