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
#include <actions/ActivateTurnoutAction.h>
#include <actions/MoveToAction.h>
#include "displayservice.h"
#include "Train.h"
#include "PerformanceMonitor.h"

RailroadLogicService::RailroadLogicService(ITrainController *ctrl, IVisionService* vision, IDisplayService* display, Configuration* configuration, AudioService* audioService)
{
    this->actionRunner = new ActionRunner();
    this->actionRunner->setRailroadService(this);
    this->resumeSpeed = 0;
    this->controller = ctrl;
    this->vision = vision;
    this->display = display;
    this->configuration = configuration;
    this->audioService = audioService;

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
    if (this->train) delete this->train;
    this->train = nullptr;
    delete this->actionRunner;
    this->actionRunner = nullptr;
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

RailroadLogicService::SplitterSearchNode RailroadLogicService::getConnectingSplitters(const QString& destinationTrack, const QString& currentTrack, const QVector<QString>& visitedTracks)
{
    // The very first node only acts as a container for all nodes. Since we can be on a track
    // with several turnouts on it, those nodes would mean something, but not the first node.
    RailroadLogicService::SplitterSearchNode node;
    node.valid = false;
    node.end = false;
    node.sa = nullptr;
    node.targetTrack = currentTrack; // this node represents a splitter that will take us to the target track

    if (currentTrack.isEmpty() || visitedTracks.contains(currentTrack))
    {
        qDebug() << "Deadend: " << visitedTracks;
        return node;
    }

    if (currentTrack == destinationTrack)
    {
        node.end = true;
        node.valid = true;
        qDebug() << "Valid Path: " << visitedTracks;
        return node;
    }

    QVector<QString> visited = visitedTracks;
    visited << currentTrack;
    for (auto sa : this->configuration->getSplitterAnnotations())
    {
        if (sa->getTrack1() == currentTrack)
        {
            RailroadLogicService::SplitterSearchNode n = this->getConnectingSplitters(destinationTrack, sa->getTrack2(), visited);
            if (n.valid)
            {
                // TODO: Do I need to go clockwise or counterclockwise to go from track1 to track2?
                n.sa = sa;
                node.children.append(n);
                node.valid = true;
            }
                    
        } 
        else if (sa->getTrack2() == currentTrack)
        {
            RailroadLogicService::SplitterSearchNode n = this->getConnectingSplitters(destinationTrack, sa->getTrack1(), visited);
            if (n.valid)
            {
                // TODO: Do I need to go clockwise or counterclockwise to go from track2 to track1?
                n.sa = sa;
                node.children.append(n);
                node.valid = true;
            }
        }
    }

    return node;
}

int RailroadLogicService::getDistanceOnTrack(Track* track, QPoint source, QPoint destination, bool direction)
{
    QPolygon poly = track->extractSegment(source, destination, direction);

    return Functions::perimeter(poly);
}

// Going from T1 -> T2 in a clockwise splitter: reverse
// Going from T2 -> T1 in a clockwise splitter:  forward
// Going from T1 -> T2 in a counter-clockwise splitter: forward
// Going from T2 -> T1 in a counter-clockwise splitter: reverse
int RailroadLogicService::findShortestPath(SplitterSearchNode& node, TrainPosition currentPos, int accumulatedDistance, int level)
{
    if (node.end)
    {
        QString str;
        QTextStream(&str) << "Total distance: " << accumulatedDistance; 
        qDebug() << str.rightJustified(str.size()+(level*4),' ');
        return accumulatedDistance;
    }

    int shortest = 1000000;
    SplitterSearchNode shortestChild;
    for (auto& it : node.children)
    {
        QString destinationTrack = it.targetTrack;
        QString sourceTrack = (destinationTrack == it.sa->getTrack1())?it.sa->getTrack2():it.sa->getTrack1();
        bool reverseThroughSplitter;

        if (destinationTrack==it.sa->getInputTrack() && sourceTrack == it.sa->getTrack2() && it.sa->getClockWise()) reverseThroughSplitter = false;
        if (destinationTrack!=it.sa->getInputTrack() && sourceTrack == it.sa->getTrack2() && it.sa->getClockWise()) reverseThroughSplitter = true;
        if (destinationTrack==it.sa->getInputTrack() && sourceTrack == it.sa->getTrack2() && !it.sa->getClockWise()) reverseThroughSplitter = true;
        if (destinationTrack!=it.sa->getInputTrack() && sourceTrack == it.sa->getTrack2() && !it.sa->getClockWise()) reverseThroughSplitter = false;
        if (destinationTrack==it.sa->getInputTrack() && sourceTrack == it.sa->getTrack1() && !it.sa->getClockWise()) reverseThroughSplitter = true;
        if (destinationTrack!=it.sa->getInputTrack() && sourceTrack == it.sa->getTrack1() && !it.sa->getClockWise()) reverseThroughSplitter = false;
        if (destinationTrack==it.sa->getInputTrack() && sourceTrack == it.sa->getTrack1() && it.sa->getClockWise()) reverseThroughSplitter = false;
        if (destinationTrack!=it.sa->getInputTrack() && sourceTrack == it.sa->getTrack1() && it.sa->getClockWise()) reverseThroughSplitter = true;

        TrainPosition nextPos;
        nextPos.point = it.sa->getPosition();
        nextPos.track = this->configuration->getTrack(destinationTrack);
        int d;
        bool loops = currentPos.track->loops(); 
        bool reverseToReachSplitter;
        if (loops)
        {
            // The splitter we want to reach can be reached by going forward or backwards. Find
            // Which way is the fastest.
            int tmp1 = this->getDistanceOnTrack(currentPos.track, currentPos.point, nextPos.point,false);
            int tmp2 = this->getDistanceOnTrack(currentPos.track, currentPos.point, nextPos.point,true);
            if (tmp1<tmp2) 
            {
                // We should move forward to reach the splitter
                d = tmp1; 
                reverseToReachSplitter = false;
            }
            else
            {
                // We should move backwards to reach the splitter
                d=tmp2;
                reverseToReachSplitter = true;
            }
        }
        else
        {
            // If the track doesn't loop, then we reach the splitter through the same direction we will go through it.
            d = this->getDistanceOnTrack(currentPos.track, currentPos.point, nextPos.point, reverseThroughSplitter);
            reverseToReachSplitter = reverseThroughSplitter;
        }

        d += accumulatedDistance;
        int r = findShortestPath(it, nextPos, d, level+1);

        qDebug() << QString(reverseToReachSplitter?"rev to: ":"fwd to: ").rightJustified(8+(level*4),' ') << destinationTrack;
        if (r < shortest)
        {
            shortest = r;
            shortestChild = it;
            shortestChild.reverseThroughSplitter = reverseThroughSplitter;
            shortestChild.reverseToReachSplitter = reverseToReachSplitter;
        }
    }

    node.children.clear();
    node.children.append(shortestChild);

    return shortest;
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

    QVector<QString> visitedTracks;
    RailroadLogicService::SplitterSearchNode nodes = this->getConnectingSplitters(name, tp.track->getName(), visitedTracks);
    if (!nodes.valid) return;

    // Each pathfrom the root of the tree to a leaf is a series of turnouts
    // For each of those turnout, we know that it is taking us to the next track
    // through track1 or 2. the "track1" variable indicates that the next track is
    // connected on track1 of that splitter. If the next track is connected to track1,
    //  it means that the current track is connected to track2. This means we need to reverse
    //  out of that turnout 
    qDebug() << "Starting from " << tp.track->getName();

    uint32_t d = this->findShortestPath(nodes,tp, 0, 1);

    qDebug() << "Shortest path: " << d;
    QPoint lastPosition = tp.point;

    if (nodes.children.size() > 0)
    {
        SplitterSearchNode n = nodes.children[0];
        bool lastDirection;
        while (true)
        {
            if (n.children.size() > 1)
            {
                qDebug() << "WARNING: More than one path found in children list";
            }

            QString sourceTrack = (n.targetTrack == n.sa->getTrack1())?n.sa->getTrack2():n.sa->getTrack1();

            bool exitingSplitter = (n.reverseToReachSplitter && !n.sa->getClockWise()) ||
                                   (!n.reverseToReachSplitter && n.sa->getClockWise());
            bool reverseToReachTrack = (!n.sa->getClockWise() && exitingSplitter) || (n.sa->getClockWise() && exitingSplitter);
            bool fwdToSplitter = !n.reverseToReachSplitter;

            bool stopAfter = exitingSplitter;

            // First action is to reach the splitter
            if (stopAfter)
            {
                this->actionRunner->addAction(new MoveToAction(n.sa->getPosition(),
                            sourceTrack,
                            n.sa->getInputTrack(),
                            n.reverseToReachSplitter,
                            SPLITTER_RADIUS,
                            MoveToAction::StopPosition::After));

                lastDirection = n.reverseToReachSplitter;
                // If this still doesn't bring us to where we wanted to go, it means we have to enter
                // the splitter in the opposite direction to reach the other track
                if (n.sa->getInputTrack() != n.targetTrack)
                {
                    this->actionRunner->addAction(new MoveToAction(n.sa->getPosition(),
                                n.sa->getInputTrack(),
                                n.targetTrack,
                                !n.reverseToReachSplitter,
                                SPLITTER_RADIUS,
                                MoveToAction::StopPosition::After));
                    lastDirection = !n.reverseToReachSplitter;
                }
            }
            else
            {
                this->actionRunner->addAction(new MoveToAction(n.sa->getPosition(),
                            sourceTrack,
                            n.targetTrack,
                            n.reverseToReachSplitter,
                            SPLITTER_RADIUS,
                            MoveToAction::StopPosition::After));
                lastDirection = n.reverseToReachSplitter;
            }
            lastPosition = n.sa->getPosition();

            if (n.end) break;
            n = n.children[0];
        }
    }

    Track* finalTrack = this->configuration->getTrack(this->waypoint.track->getName());
    //if (finalTrack->loops())
    {
        int revDistance = this->getDistanceOnTrack(finalTrack, lastPosition, this->waypoint.point, true);
        int fwdDistance = this->getDistanceOnTrack(finalTrack, lastPosition, this->waypoint.point, false);
        this->actionRunner->addAction(new MoveToAction(this->waypoint.point,
                    this->waypoint.track->getName(),
                    this->waypoint.track->getName(),
                    (revDistance<fwdDistance),
                    DETECT_RECT,
                    MoveToAction::StopPosition::Within));
    }
    /*else
    {
        this->actionRunner->addAction(new MoveToAction(this->waypoint.point,
                    this->waypoint.track->getName(),
                    this->waypoint.track->getName(),
                    lastDirection,
                    DETECT_RECT,
                    MoveToAction::StopPosition::Within));
    }*/

    this->actionRunner->run();

}


// Enabled == true means that relay1 will be activated and the splitter will enable forking
//            forking would be enabled wheter backing off or going forward (getting out of track or into)
void RailroadLogicService::activateSplitter(SplitterAnnotation* sa, bool enabled)
{
    if (!sa) return;

    int relay = enabled?sa->getRelay(0):sa->getRelay(1);

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

    this->findTurnouts();
    this->updateAnnotations();
    //TODO: build image here and pass it instead of passing a list that we need to loop through again
    this->vision->setTrackMask(tracksPoly);
}

void RailroadLogicService::updateAnnotations()
{
/*QImage image(VIDEO_WIDTH, VIDEO_HEIGHT, QImage::Format_RGB32);
QPainter p(&image);
p.setPen(QPen(Qt::blue,5));*/

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

/*QPoint center=a->getPosition();
QPolygon test(QRect(center-QPoint(ANNOTATION_RADIUS,ANNOTATION_RADIUS),center+QPoint(ANNOTATION_RADIUS,ANNOTATION_RADIUS)),true);
QPolygon test2 = test.intersected(this->configuration->getTracks()[0]->getPolygon());
p.drawPolygon(test2);*/

    }
//DisplayService::debugPixmap->setPixmap(QPixmap::fromImage(image).copy());
}

void RailroadLogicService::findTurnouts()
{
//QImage image(VIDEO_WIDTH, VIDEO_HEIGHT, QImage::Format_RGB32);
//QPainter p(&image);

    // Remove all splitters from display and then delete them from config
    for (Annotation* a : this->configuration->getAnnotations())
    {
        SplitterAnnotation* sa = dynamic_cast<SplitterAnnotation*>(a);
        if (!sa) continue;
        if (!this->display) continue;
        this->display->removeItem(sa->getName());
    }
    this->configuration->clearSplitterAnnotations();

    // Rebuild the splitters list
    auto tracks = this->configuration->getTracks();
    for (int index1 = 0; index1 < tracks.size(); index1++)
    {
        QPolygon poly1 = tracks[index1]->getPolygon();
        if (poly1.size() < 2) continue;

        QPoint lastPoint = poly1.last();
        QPoint firstPoint = poly1.first();

        for (int index2 = 0; index2 < tracks.size(); index2++)
        {
            if (index1 == index2) continue;
            QPolygon poly2 = tracks[index2]->getPolygon();
            if (poly2.size() < 2) continue;


            bool foundStart = false;
            bool foundEnd = false;
            int curveIndex = 0;
            for (QPoint curvePoint : poly2)
            {
                int ci = curveIndex;
                curveIndex++;

                // TODO: this assumes that tracks are at least 10 points longs
                if (ci < 5 || ci > (poly2.size()-6)) continue;

                if (!foundStart)
                {
                    QLineF intersectingLine(curvePoint,firstPoint);
                    if (intersectingLine.length() <60)
                    {
                        // We found a turnout. Since this is the start of the polygon then
                        // we can assume it is the entry to the track. So this turnout
                        // is going counterclockwise
                        //  <---------------
                        //      /
                        //  <----
                        //
                        // TODO: this assumes that tracks are at least 10 points longs
                        QPoint curveZ = curvePoint;
                        QPoint curveA = poly2.at(ci-5);
                        QPoint curveB = poly2.at(ci+5);
                        QLineF lineZA(curveZ,curveA);
                        QLineF lineZB(curveZ,curveB);
                        double angleAZB = lineZA.angleTo(lineZB);
                        QPoint pointC = poly1.at(5);
                        QLineF lineZC(poly1.first(),pointC);
                        double angleAZC = QLineF(poly1.first(),curveA).angleTo(lineZC);
                        qDebug() << "Angle AZB= " << angleAZB << ", AZC= " << angleAZC;

                        foundStart = true;
                        SplitterAnnotation* sa = new SplitterAnnotation();
                        // The angle that's the closest to 180 is the one that goes straight and will be track1
                        if (abs(angleAZB-180) < abs(angleAZC-180))
                        {
                            sa->setTrack1(tracks[index2]->getName());
                            sa->setTrack2(tracks[index1]->getName());
                        }
                        else
                        {
                            sa->setTrack1(tracks[index1]->getName());
                            sa->setTrack2(tracks[index2]->getName());
                        }
                        sa->setInputTrack(tracks[index2]->getName());
                        sa->setClockWise(false);
                        sa->setPosition(curvePoint);
                        sa->setName(QUuid::createUuid().toString());
                        this->configuration->addAnnotation(sa);
                        qDebug() << "Found splitter at " << curvePoint << "Track1 = " << sa->getTrack1();
                    }
                }
                if (!foundEnd)
                {
                    QLineF intersectingLine(curvePoint,lastPoint);
                    if (intersectingLine.length() <60)
                    {
                        // We found a turnout. Since this is the end of the polygon then
                        // we can assume it is the exit to the track. So this turnout
                        // is going clockwise
                        //  <---------------
                        //      \
                        //       ----
                        //
                        // TODO: this assumes that tracks are at least 10 points longs
                        QPoint curveZ = curvePoint;
                        QPoint curveA = poly2.at(ci+5);  // curveA and curveB are opposite since we go clockwise here
                        QPoint curveB = poly2.at(ci-5);
                        QLineF lineZA(curveZ,curveA);
                        QLineF lineZB(curveZ,curveB);
                        double angleAZB = lineZA.angleTo(lineZB);
                        QPoint pointC = poly1.at(poly1.size()-6);
                        QLineF lineZC(poly1.last(),pointC);
                        double angleAZC = QLineF(poly1.last(),curveA).angleTo(lineZC);
                        qDebug() << "Angle AZB= " << angleAZB << ", AZC= " << angleAZC;

                        foundEnd = true;
                        SplitterAnnotation* sa = new SplitterAnnotation();
                        // The angle that's the closest to 180 is the one that goes straight and will be track1
                        if (abs(angleAZB-180) < abs(angleAZC-180))
                        {
                            sa->setTrack1(tracks[index2]->getName());
                            sa->setTrack2(tracks[index1]->getName());
                        }
                        else
                        {
                            sa->setTrack1(tracks[index1]->getName());
                            sa->setTrack2(tracks[index2]->getName());
                        }
                        sa->setInputTrack(tracks[index2]->getName());
                        sa->setClockWise(true);
                        sa->setPosition(curvePoint);
                        sa->setName(QUuid::createUuid().toString());
                        this->configuration->addAnnotation(sa);
                        qDebug() << "Found splitter at " << curvePoint << "Track1 = " << sa->getTrack1();
                    }
                }
                if (foundStart && foundEnd)
                {
                    break;
                }

            }
        }
    }
//DisplayService::debugPixmap->setPixmap(QPixmap::fromImage(image).copy());
}
