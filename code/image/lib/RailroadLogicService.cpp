#include "RailroadLogicService.h"
#include "constants.h"
#include "crossroadannotation.h"
#include "splitterannotation.h"
#include "Functions.h"
#include <QTimer>
#include <QUuid>
#include <QPoint>
#include <QDebug>
#include "ActionRunner.h"
#include <actions/MoveToAction.h>
#include <actions/MoveToSplitterAction.h>
#include <actions/ChangeTrackAction.h>
#include <actions/InitSplittersAction.h>
#include "displayservice.h"
#include "Train.h"
#include "PerformanceMonitor.h"

RailroadLogicService::RailroadLogicService(ITrainController *ctrl, IVisionService* vision, IDisplayService* display, Configuration* configuration, AudioService* audioService)
{
    this->actionRunner = new ActionRunner();
    this->actionRunner->setRailroadService(this);
    this->actionRunner->setConfiguration(configuration);
    this->resumeSpeed = 0;
    this->controller = ctrl;
    this->vision = vision;
    this->display = display;
    this->configuration = configuration;
    this->audioService = audioService;
    this->railroad = new Railroad();

    this->train = nullptr;

    connect(this->vision, SIGNAL(locomotivePositionChanged(CVObject)), this, SLOT(on_locomotive_changed(CVObject)));
    connect(this->vision, SIGNAL(markerFound(DetectedMarker)), this, SLOT(on_marker_found(DetectedMarker)));
    connect(this->display, SIGNAL(waypointSet(QPoint)), this, SLOT(on_waypoint_set(QPoint)));

    this->controller->setSpeed(0);

    // Enable detection of annotations only for 1 minute
    this->vision->enableAnnotationDetection(true);
    QTimer::singleShot(1000*60*1, [=]() {
        this->vision->enableAnnotationDetection(false);
    });
}

RailroadLogicService::~RailroadLogicService()
{
    if (this->railroad) delete this->railroad;
    this->railroad = nullptr;

    if (this->train) delete this->train;
    this->train = nullptr;

    delete this->actionRunner;
    this->actionRunner = nullptr;
}

Railroad* RailroadLogicService::getRailroad()
{
    return this->railroad;
}

void RailroadLogicService::on_waypoint_set(QPoint p)
{
    this->setWaypoint(p);
}


void RailroadLogicService::on_locomotive_changed(CVObject obj)
{
    PerformanceMonitor::tic("RailroadLogicService::on_locomotive_changed");


    if (this->train) delete this->train;
    train = new Train(obj);
    for (auto w : this->vision->wagons())
    {
        train->addWagon(w);
    }

    this->checkAnnotations();
    this->actionRunner->onTrainMoved(this->train);

    ////////////////////////////////////
    // Update display
    ////////////////////////////////////
    QGraphicsPolygonItem* loco = this->display->polygonItem("locomotive");
    if (loco)
    {
        //QVariant v;
        //v.setValue(obj);
        //loco->setData(1,v);
        loco->setPolygon(train->getLocomotive());
    }

    int i = 0;
    for (QPolygon poly : train->getLinkedWagons())
    {
        QString name;
        QTextStream(&name) << "wagon" << i;
        QGraphicsPolygonItem* w = this->display->polygonItem(name);
        if (!w) continue;
        //v.setValue(wagons[i]);
        //w->setData(1,v);
        w->setPolygon(poly);
        w->setBrush(QBrush(QColor(255,0,255,64)));

        i++;
    }
    for (QPolygon poly : train->getUnlinkedWagons())
    {
        QString name;
        QTextStream(&name) << "wagon" << i;
        QGraphicsPolygonItem* w = this->display->polygonItem(name);
        if (!w) continue;
        //v.setValue(wagons[i]);
        //w->setData(1,v);
        w->setPolygon(poly);
        w->setBrush(QBrush(QColor(255,255,0,64)));

        i++;
    }

    PerformanceMonitor::toc("RailroadLogicService::on_locomotive_changed");
}

void RailroadLogicService::checkAnnotations()
{
    if (!this->train) return;
    QVector<Annotation*> annotations = this->configuration->getAnnotations();
    QVector<Annotation*> annotationsInRange = train->inRange(annotations);

    int speed = this->controller->getSpeed();


    bool shouldPlayCrossingSound = false;
    bool shouldSlowDown = false;

    for (Annotation* a : annotationsInRange)
    {
        // We remove it from the other list because we will use that other list as the list of annotations
        // that are NOT in range
        annotations.removeAll(a);

        bool hasChanged = a->setInRange(true);

        auto sa = dynamic_cast<SplitterAnnotation*>(a);
        if (sa)
        {
            shouldSlowDown = true;
            if (hasChanged)
            {
                this->actionRunner->onEnterTurnout(sa);
            }
        }

        if (dynamic_cast<CrossRoadAnnotation*>(a))
        {
            shouldSlowDown = true;
            shouldPlayCrossingSound = true;
            if (hasChanged)
            {
                this->controller->setRelay(a->getRelay(0), true);
            }
        }
    }

    if (this->audioService)
    {
        if (shouldPlayCrossingSound)
        {
            this->audioService->playCrossing();
        }
        else
        {
            this->audioService->stopCrossing();
        }
    }

    if (shouldSlowDown)
    {
        if (speed > SPEED_SLOW) this->controller->setSpeed(SPEED_SLOW);
    }
    else
    {
        if (speed < resumeSpeed) this->controller->setSpeed(resumeSpeed);
    }

    // Close all other relays that should be closed
    for (Annotation* a : annotations)
    {
        bool hasChanged = a->setInRange(false);
        if (!hasChanged) continue;

        if (dynamic_cast<CrossRoadAnnotation*>(a))
        {
            this->controller->setRelay(a->getRelay(0), false);
        }

        auto sa = dynamic_cast<SplitterAnnotation*>(a);
        if (sa)
        {
            this->actionRunner->onLeaveTurnout(sa);
        }
    }

}

void RailroadLogicService::on_marker_found(DetectedMarker m)
{
    if (m.code == MARKER_TYPE_CROSSING)
    {
        bool existsAlready = false;
        for (auto a : this->configuration->getCrossroadsAnnotations())
        {
            if ((m.pos - a->getPosition()).manhattanLength() < 100)
            {
                existsAlready = true;
                break;
            }
        }
        if (!existsAlready)
        {
            qDebug() << "Marker found at " << m.pos;
            CrossRoadAnnotation *ca = new CrossRoadAnnotation();
            ca->setPosition(m.pos);
            ca->setName(QUuid::createUuid().toString());
            this->configuration->addAnnotation(ca);
            this->updateAnnotations();
        }
    }

}

void RailroadLogicService::startTrainForward()
{
    this->controller->setDirection(false);
    this->controller->setSpeed(SPEED_NORMAL);
    this->resumeSpeed = SPEED_NORMAL;
}

void RailroadLogicService::startTrainReverse()
{
    this->controller->setDirection(true);
    this->controller->setSpeed(SPEED_NORMAL);
    this->resumeSpeed = SPEED_NORMAL;
}

void RailroadLogicService::stopTrain()
{
    this->controller->setSpeed(0);
    this->resumeSpeed = 0;
}

void RailroadLogicService::stopAll()
{
    this->actionRunner->abort();
    this->stopTrain();
}

void RailroadLogicService::setWaypoint(QPoint w)
{
    this->waypoint = this->findClosestPoint(w);
    qDebug() << "Waypoint's closest point on track is " << this->waypoint.point << " on track " << this->waypoint.track->getName(); 
    this->display->setWaypoint(w,this->waypoint.point);
}

TrainPosition RailroadLogicService::findClosestPoint(QPoint point)
{
    TrainPosition tp;

    int lastDistance = 100000;

    for (Track* t : this->configuration->getTracks())
    {
        QPolygon poly = t->getPolygon();
        QPoint p = t->findClosestPoint(point);
        QPoint delta = (p-point);
        int d = sqrt((delta.x()*delta.x())+(delta.y()*delta.y()));
        if (d < lastDistance)
        {
            lastDistance = d;
            tp.point = p;
            tp.track = t;
        }
    }

    return tp;
}

ActionRunner* RailroadLogicService::getActionRunner()
{
    return this->actionRunner;
}

QPoint RailroadLogicService::getTrackWaypoint()
{
    return this->waypoint.point;
}

void RailroadLogicService::gotoWaypoint()
{
    TrainPosition tp = this->findClosestPoint(this->train->getPosition());
    qDebug() << "current point on track is " << tp.point << " on track " << tp.track->getName(); 

    QString name = this->waypoint.track->getName();
    auto path = this->railroad->findShortestPath(tp.point, this->waypoint.point);

    // Predetermine all splitters that will need to be set
    auto it = path.begin();
    InitSplittersAction *ia = new InitSplittersAction();
    while (it != path.end())
    {
        PathInfo pi = *it;
        it++;
        if (it == path.end()) break;

        GraphEdge<TrackSection>* edge = (GraphEdge<TrackSection>*)pi.edge;
        GraphNode<SplitterAnnotation*>* dstNode = (GraphNode<SplitterAnnotation*>*)(pi.direction?edge->node1:edge->node2);
        if (!dstNode->data) continue;

        SplitterAnnotation* sa = dstNode->data;
        QString sourceTrack = edge->data.trackName;
        QString targetTrack = ((GraphEdge<TrackSection>*)(*it).edge)->data.trackName;

        bool commingFromT0 = (pi.direction == sa->getClockWise());
        QString track;
        if (commingFromT0)
        {
            track = targetTrack;
        }
        else
        {
            track = sourceTrack;
        }

        ia->addTrackInfo(sa, track);
        
    }
    this->actionRunner->addAction(ia);

    it = path.begin();
    while (1)
    {
        PathInfo pi = *it;
        GraphEdge<TrackSection>* edge = (GraphEdge<TrackSection>*)pi.edge;
        //GraphNode<SplitterAnnotation*>* sourceNode = (GraphNode<SplitterAnnotation*>*)(pi.direction?edge->node2:edge->node1);
        GraphNode<SplitterAnnotation*>* dstNode = (GraphNode<SplitterAnnotation*>*)(pi.direction?edge->node1:edge->node2);

        it++;
        if (it == path.end())
        {
            // This is the final destination. A special case.
            this->actionRunner->addAction(new MoveToAction(this->waypoint.track->getName(),
                        this->waypoint.point,
                        DETECT_RECT,
                        pi.direction));
            break;
        }
        QString sourceTrack = edge->data.trackName;
        QString targetTrack = sourceTrack;
        if (it!=path.end()) targetTrack = ((GraphEdge<TrackSection>*)(*it).edge)->data.trackName;

        bool commingFromT0 = pi.direction == dstNode->data->getClockWise();
        this->actionRunner->addAction(new MoveToSplitterAction(dstNode->data, sourceTrack, targetTrack, pi.direction));
        this->actionRunner->addAction(new ChangeTrackAction(dstNode->data, sourceTrack, targetTrack, commingFromT0));
    }

    this->actionRunner->run();
}

// Enabled == true means that relay1 will be activated and the splitter will enable forking
//            forking would be enabled wheter backing off or going forward (getting out of track or into)
void RailroadLogicService::activateSplitter(SplitterAnnotation* sa, bool enabled)
{
    if (!sa) return;

    int relay = -1;
    if (enabled)
    {
        relay = sa->getRelay(0);
        sa->setActiveTrack(sa->getTrack2());
    }
    else
    {
        relay = sa->getRelay(1);
        sa->setActiveTrack(sa->getTrack2());
    }

    qDebug() << "Activate relay " << relay << " for turnout " << sa->getName();
    this->controller->triggerRelay(relay);
}


void RailroadLogicService::updateTracks()
{
    QVector<QPolygon> tracksPoly;
    for (Track* t : this->configuration->getTracks())
    {
        QGraphicsPathItem* path = this->display->track(t->getName());
        if (!path)
        {
            this->display->createTrackItem(t->getName(), IDisplayService::ViewType::All);
            path = this->display->track(t->getName());
            QPen trackPen;
            trackPen.setColor(QColor(255,255,0,32));
            trackPen.setWidth(TRACK_WIDTH);
            path->setPen(trackPen);
        }
        path->setZValue(30); //TODO: use enum for that value
        QPainterPath p;
        p.addPolygon(t->getPolygon());
        path->setPath(p);
        tracksPoly.push_back(t->getPolygon());
    }

    this->updateTurnouts();
    this->updateAnnotations();
    //TODO: build image here and pass it instead of passing a list that we need to loop through again
    this->vision->setTrackMask(tracksPoly);
}

void RailroadLogicService::updateAnnotations()
{
    for (Annotation* a : this->configuration->getAnnotations())
    {
        int radius = SPLITTER_RADIUS;
        if (dynamic_cast<CrossRoadAnnotation*>(a)) radius = CROSSROAD_RADIUS;
        QGraphicsItem* g = this->display->item(a->getName());
        if (!g)
        {
            g = this->display->createAnnotationItem(a->getName(), IDisplayService::ViewType::All, a->pixmapName(), radius, true);
        }

        QGraphicsEllipseItem* e = dynamic_cast<QGraphicsEllipseItem*>(g);
        if (!e) continue;

        e->setPos(a->getPosition()-e->rect().center().toPoint());
        e->setZValue(50); //TODO: use enum for that value
        e->setData(1,a->getName());
    }
}

void RailroadLogicService::updateTurnouts()
{
    this->railroad->setTracks(this->configuration->getTracks());
    this->configuration->clearSplitterAnnotations();
    // Remove all splitters from display and then delete them from config
    for (SplitterAnnotation* sa : this->railroad->getTurnouts()) 
    {
        this->configuration->addAnnotation(sa);

        if (!this->display) continue;
        this->display->removeItem(sa->getName());
    }
}
