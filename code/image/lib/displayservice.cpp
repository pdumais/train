#include "displayservice.h"
#include "cvobject.h"
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QGraphicsVideoItem>
#include <QGraphicsItemGroup>
#include <QTimer>
#include <unistd.h>
#include "qdragdropgraphicsscene.h"
#include "constants.h"

QGraphicsPixmapItem* DisplayService::debugPixmap;


DisplayService::DisplayService()
{
    this->operationScene = new QDragDropGraphicsScene();
    this->learningScene = new QDragDropGraphicsScene();

    connect(this->operationScene, SIGNAL(selectionChanged()), this, SLOT(on_operation_item_selected()));
    connect(this->operationScene, SIGNAL(mouseClick(QPoint)), this, SIGNAL(waypointSet(QPoint)));
}

DisplayService::~DisplayService()
{
}


void DisplayService::init(QGraphicsView* view)
{
    this->view = view;

    this->view->setSceneRect(0,0,1920,1080);
    this->videoItem = this->createVideoItem();

    this->camera = new QCamera("/dev/video0");
    camera->setCaptureMode(QCamera::CaptureViewfinder);
    camera->setViewfinder(this->videoItem);

    //camera->viewfinderSettings().setResolution(1920,1080);
    camera->viewfinderSettings().setMaximumFrameRate(15);
    this->camera->start();

    this->debugPixmap = new QGraphicsPixmapItem();
    this->createTrackItem("LearningTrack", ViewType::Learning);
    this->createPixmapItem("_waypoint", ViewType::Operation, ":/waypoint.png");
    this->addGraphicsItem("_waypointline", new QGraphicsLineItem(), ViewType::Operation);
    this->addGraphicsItem("_waypointtext", new QGraphicsSimpleTextItem(), ViewType::Operation);

    this->setViewType(ViewType::Operation);

    QPen trackPen;
    trackPen.setColor(QColor(0,255,0,64));
    trackPen.setWidth(TRACK_WIDTH);
    this->track("LearningTrack")->setPen(trackPen);

    for (int i = 0; i < 20; i++)
    {
        QString name;
        QTextStream(&name) << "wagon" << i;
        this->createWagonItem(name,DisplayService::ViewType::All);
    }
}

void DisplayService::on_operation_item_selected()
{
    auto items = this->operationScene->selectedItems();
    if (items.size() < 1) return;

    QVariant v = items[0]->data(1);
    if (v.userType() == QMetaType::QString)
    {
        qDebug() << v.value<QString>();
    }

}


void DisplayService::removeItem(QString name)
{
    if (!this->items.contains(name)) return;

    auto item = this->items[name].item;
    this->items.remove(name);
    this->operationScene->removeItem(item);
    this->learningScene->removeItem(item);

    //TODO: do we need to delete items within a group?
    delete item;
}

void DisplayService::setWaypoint(QPoint w, QPoint trackPoint)
{
    this->wayPoint = w;
    this->wayPointEnd = trackPoint;
    this->updateWayPoint();
}

void DisplayService::updateWayPoint()
{
    QGraphicsPixmapItem* w = dynamic_cast<QGraphicsPixmapItem*>(this->item("_waypoint"));
    QGraphicsLineItem* l = dynamic_cast<QGraphicsLineItem*>(this->item("_waypointline"));
    QGraphicsSimpleTextItem* text = dynamic_cast<QGraphicsSimpleTextItem*>(this->item("_waypointtext"));
    w->setPos(this->wayPoint);
    QPen pen(QColor(200,100,100));
    pen.setWidth(4);
    pen.setStyle(Qt::DashLine);
    l->setPen(pen);
    QSize pixSize = w->pixmap().size()/2;
    QLine line = QLine(this->wayPoint+QPoint(pixSize.width(),pixSize.height()),this->wayPointEnd);
    l->setLine(line);

    // TODO: should show distance in cm instead of pixels. We know how much pixels there are in a wagon
    //       because of the constant WAGON_WIDTH so we could calculate that assuming we know the wagon size in cm
    QLineF linef(line);
    QString str;
    QTextStream(&str) << "[" << int(linef.length()) << "]";
    text->setText(str);
    text->setPen(QColor(Qt::red));
    text->setRotation(-linef.angle());
    text->font().setPixelSize(24);
    text->setPos(((line.p2()-line.p1())/2)+line.p1());
    
}

QGraphicsVideoItem* DisplayService::createVideoItem()
{
    QGraphicsVideoItem* g = new QGraphicsVideoItem();

    g->setSize(QSize(1920,1080));
    g->setAspectRatioMode(Qt::IgnoreAspectRatio);
    g->setZValue(-5);

    return g;
}

void DisplayService::start()
{
}

void DisplayService::setViewType(ViewType vt)
{
    qDebug() << "VisionController::setViewType " << int(vt);

    QGraphicsScene *scene = nullptr;
    QGraphicsVideoItem *video = nullptr;
    switch (vt)
    {
    case ViewType::Operation:
        scene = this->operationScene;
        break;
    case ViewType::Learning:
        scene = this->learningScene;
        break;
    case ViewType::All:
        scene = nullptr;
        video = nullptr;
        break;
    }

    if (scene == nullptr) return;
    // TODO: add tracks to new scene

    if (this->view->scene())
    {
        this->view->scene()->removeItem(this->videoItem);
        this->view->scene()->clearSelection();
    }
    scene->addItem(this->videoItem);
    scene->addItem(this->debugPixmap);
    for (auto &it : this->items)
    {
        if (it.viewType == ViewType::All || it.viewType == vt)
        {
            scene->addItem(it.item);
        }
    }

    this->view->setScene(scene);
}

void DisplayService::setProbe(QVideoProbe *probe)
{
    if (!probe->setSource(this->camera))
    {
        qDebug() << "Can't attach probe to camera";
    }
}


QGraphicsPolygonItem* DisplayService::createLocomotiveItem(QString name, ViewType viewType)
{
    QGraphicsPolygonItem* r = new QGraphicsPolygonItem();
    r->setPos(0,0);
    r->setPen(Qt::NoPen);
    r->setBrush(QBrush(QColor(255,0,0,64)));
    this->addGraphicsItem(name,r,viewType);
    return r;
}

QGraphicsPolygonItem* DisplayService::createWagonItem(QString name, ViewType viewType)
{
    QGraphicsPolygonItem* r = new QGraphicsPolygonItem();
    r->setPos(0,0);
    r->setPen(Qt::NoPen);
    r->setBrush(QBrush(QColor(255,0,255,64)));

    this->addGraphicsItem(name,r,viewType);
    return r;
}

QGraphicsPathItem* DisplayService::createTrackItem(QString name, ViewType viewType)
{
    QGraphicsPathItem *item = new QGraphicsPathItem();
    this->addGraphicsItem(name,item,viewType);
    return item;
}

QGraphicsPixmapItem* DisplayService::createPixmapItem(QString name, ViewType viewType, QString fileName, bool selectable)
{
    QImage img(fileName);
    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(img).scaled(QSize(64,64)));
    this->addGraphicsItem(name,item,viewType, selectable);
    return item;
}

QGraphicsEllipseItem* DisplayService::createAnnotationItem(QString name, ViewType viewType, QString fileName, int radius, bool selectable)
{
    QImage img(fileName);
    QGraphicsEllipseItem* e = new QGraphicsEllipseItem(QRect(0,0,radius*2,radius*2));
    e->setBrush(QBrush(QColor(0,190,240,40)));
    e->setPen(Qt::NoPen);
    QGraphicsPixmapItem* px = new QGraphicsPixmapItem(QPixmap::fromImage(img).scaled(QSize(64,64)),e);
    px->setPos(radius-32,radius-32);
    this->addGraphicsItem(name,e,viewType, selectable);
    return e;
}

QGraphicsItem* DisplayService::addGraphicsItem(QString name, QGraphicsItem* item, ViewType viewType, bool selectable)
{
    ItemInfo itemInfo;
    itemInfo.item = item;
    itemInfo.viewType = viewType;
    itemInfo.name = name;
    this->items[name] = itemInfo;
    //TODO: only add in current scene if it is marked for that scene
    if (this->view->scene()) this->view->scene()->addItem(item);
    if (selectable) item->setFlag(QGraphicsItem::ItemIsSelectable);

    return item;
}

QGraphicsPathItem* DisplayService::track(QString name)
{
    if (!this->items.contains(name)) return nullptr;

    return dynamic_cast<QGraphicsPathItem*>(this->items[name].item);

}

QGraphicsItem* DisplayService::item(QString name)
{
    if (!this->items.contains(name)) return nullptr;

    return this->items[name].item;
}

QGraphicsPolygonItem* DisplayService::polygonItem(QString name)
{
    return dynamic_cast<QGraphicsPolygonItem*>(this->item(name));
}
