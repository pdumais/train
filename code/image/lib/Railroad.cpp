#include "Railroad.h"
#include <QLineF>
#include <QDebug>
#include <QUuid>
#include "constants.h"
#include "Functions.h"
#include <math.h>


Railroad::Railroad()
{
    this->tracksGraph = nullptr;
}

Railroad::~Railroad()
{
}

void Railroad::setTracks(QVector<Track*> val)
{
    this->tracks = val;
    this->findTurnouts();
    this->buildGraph();
}

Graph* Railroad::getGraph()
{
    return this->tracksGraph;
}

QVector<SplitterAnnotation*> Railroad::getTurnouts()
{
    return this->turnouts;
}

int Railroad::getClosestTurnout(SplitterAnnotation* current, QString track, bool reverse, SplitterAnnotation*& splitter, QPolygon& section)
{
    Track* currentTrack = nullptr;
    for (Track* t : this->tracks)
    {
        if (t->getName() != track) continue;
        currentTrack = t;
        break;
    }
    if (!currentTrack) return -1;

    section.clear();
    int index = currentTrack->findClosestIndex(current->getPosition());
    QPolygon poly = currentTrack->getPolygon();
    double distance = 0;

    int initialIndex = index;
    QPoint lastPoint = poly[initialIndex];
    while(true)
    {
        if (index == -1 && reverse)
        {
            if (currentTrack->loops())
            {
                index = poly.size()-1;
            }
            else
            {
                break;
            }
        }
        else if (index == (poly.size()-1) && !reverse)
        {
            if (currentTrack->loops())
            {
                index = 0;
            }
            else
            {
                break;
            }
        }
        QPoint newPoint = poly[index];
        distance += HYPOTHENUS((newPoint-lastPoint));
        lastPoint = newPoint;

        if (reverse) index--; else index++;
        if (reverse) section.prepend(newPoint); else section.append(newPoint);


        for (auto s : this->turnouts)
        {
            if (s == current) continue;

            QPoint realPos =s->getPosition();
            if ((s->getTrack1() == currentTrack->getName() || s->getTrack2() == currentTrack->getName() || s->getInputTrack() == currentTrack->getName()) && HYPOTHENUS((realPos-newPoint)) <= (60)) //TODO: MAGIC NUMBER
            {
                //this splitter is on our track
                splitter = s;
                return int(distance);
            }

        }
        if (initialIndex == index) break;
    }

    return -1;
}

void Railroad::buildGraph()
{
    if (this->tracksGraph) delete this->tracksGraph;
    this->tracksGraph = new Graph();

    for (auto sa : this->turnouts)
    {
        QString nodeName = sa->toString();
        this->tracksGraph->addNode(nodeName.toStdString(), sa);
    }

    for (auto sa : this->turnouts)
    {
        QString track0 = sa->getInputTrack();
        QString track1 = sa->getTrack1();
        QString track2 = sa->getTrack2();
        QString nodeName;

        SplitterAnnotation* nextSa;
        TrackSection section;
        QPolygon poly;

        int cost;
        cost = this->getClosestTurnout(sa, track0, !sa->getClockWise(), nextSa, poly);
        section.polygon = poly;
        section.trackName = track0;

        if (cost != -1)
        {
            nodeName = nextSa->toString();
        }
        else
        {
            nodeName = track0+"end";
            this->tracksGraph->addNode(nodeName.toStdString(), std::any());
        }

        if (sa->getClockWise())
        {
            // if the node is clockwise and the next one is reachable thru t0, it means we go forward
            this->tracksGraph->addEdge(sa->toString().toStdString(), nodeName.toStdString(), section, cost);
        }
        else
        {
            this->tracksGraph->addEdge(nodeName.toStdString(), sa->toString().toStdString(), section, cost);
        }

        cost = this->getClosestTurnout(sa, track1, sa->getClockWise(), nextSa, poly);
        section.polygon = poly;
        section.trackName = track1;
        if (cost != -1)
        {
            nodeName = nextSa->toString();
        }
        else
        {
            nodeName = track1+"end";
            this->tracksGraph->addNode(nodeName.toStdString(), std::any());
        }

        if (sa->getClockWise())
        {
            // if the node is clockwise and the next one is reachable thru t1/t2, it means we go backwards
            this->tracksGraph->addEdge(nodeName.toStdString(), sa->toString().toStdString(), section, cost);
        }
        else
        {
            this->tracksGraph->addEdge(sa->getName().toStdString(), nodeName.toStdString(), section, cost);
        }

        cost = this->getClosestTurnout(sa, track2, sa->getClockWise(), nextSa, poly);
        section.polygon = poly;
        section.trackName = track2;
        if (cost != -1)
        {
            nodeName = nextSa->toString();
        }
        else
        {
            nodeName = track2+"end";
            this->tracksGraph->addNode(nodeName.toStdString(), std::any());
        }

        if (sa->getClockWise())
        {
            // if the node is clockwise and the next one is reachable thru t1/t2, it means we go backwards
            this->tracksGraph->addEdge(nodeName.toStdString(), sa->toString().toStdString(), section, cost);
        }
        else
        {
            this->tracksGraph->addEdge(sa->toString().toStdString(), nodeName.toStdString(), section, cost);
        }
    }
}

void Railroad::findTurnouts()
{
    // No need to delete them. They are handed off to Configuration and it will delete them
    // TODO: should consider using shared_ptr for that
    this->turnouts.clear();

    // Rebuild the splitters list
    for (int index1 = 0; index1 < this->tracks.size(); index1++)
    {
        QPolygon poly1 = this->tracks[index1]->getPolygon();
        if (poly1.size() < 2) continue;

        QPoint lastPoint = poly1.last();
        QPoint firstPoint = poly1.first();

        for (int index2 = 0; index2 < this->tracks.size(); index2++)
        {
            if (index1 == index2) continue;
            QPolygon poly2 = this->tracks[index2]->getPolygon();
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
                        this->turnouts.append(sa);
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
                        this->turnouts.append(sa);
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


}

int Railroad::calculateCost(const QPolygon& poly)
{
    double distance = 0;
    QPoint old = poly.first();
    for (QPoint newPoint : poly)
    {
        distance += HYPOTHENUS((newPoint-old));
        old = newPoint;
    }

    return int(distance);
}

//TODO: make sure that source and dest have been normalized (aka: they are points projected on track)
std::vector<PathInfo> Railroad::findShortestPath(QPoint source, QPoint dest)
{
    // we make a copy of the graph because we don't want to alter the original one
    Graph* g = new Graph(this->tracksGraph);

    // Find the edge on which source/dest is on
    int sourceEdge=-1;
    int destinationEdge=-1;
    int indexOfSource;
    int indexOfDestination;
    for (int i = 0; i < g->edgeCount(); i++)
    {
        TrackSection ts = anycast(g->getEdgeData(i));
        QPolygon section = ts.polygon;
        int n;

        n = section.indexOf(source);
        if (n != -1)
        {
            sourceEdge = i;
            indexOfSource = n;
        }

        n = section.indexOf(dest);
        if (n != -1)
        {
            destinationEdge = i;
            indexOfDestination = n;
        }
    }

    if (sourceEdge == -1 || destinationEdge == -1)
    {
        qDebug() << "Edge not found for source/destination";
        return std::vector<PathInfo>();
    }

    // TODO: this won't work if the source or destination falls exactly on a node point

    // We will remove the 2 edges and replace them by 2 edges each with a new node somewhere in the middle
    // These new nodes will be the source and destination
    TrackSection sourceSection1;
    TrackSection sourceSection2;
    TrackSection destinationSection1;
    TrackSection destinationSection2;

    sourceSection1.polygon = std::any_cast<TrackSection>(g->getEdgeData(sourceEdge)).polygon.mid(0,indexOfSource);
    sourceSection2.polygon = std::any_cast<TrackSection>(g->getEdgeData(sourceEdge)).polygon.mid(indexOfSource);
    destinationSection1.polygon = std::any_cast<TrackSection>(g->getEdgeData(destinationEdge)).polygon.mid(0,indexOfDestination);
    destinationSection2.polygon = std::any_cast<TrackSection>(g->getEdgeData(destinationEdge)).polygon.mid(indexOfDestination);
    sourceSection1.trackName = std::any_cast<TrackSection>(g->getEdgeData(sourceEdge)).trackName;
    sourceSection2.trackName = std::any_cast<TrackSection>(g->getEdgeData(sourceEdge)).trackName;
    destinationSection1.trackName = std::any_cast<TrackSection>(g->getEdgeData(destinationEdge)).trackName;
    destinationSection2.trackName = std::any_cast<TrackSection>(g->getEdgeData(destinationEdge)).trackName;


    std::string sourceNode1 = g->getEdgeNodeName(sourceEdge,true);
    std::string sourceNode2 = g->getEdgeNodeName(sourceEdge,false);
    std::string destNode1 = g->getEdgeNodeName(destinationEdge,true);
    std::string destNode2 = g->getEdgeNodeName(destinationEdge,false);
    g->removeEdge(sourceEdge);
    if (sourceEdge < destinationEdge) destinationEdge--;  // because we removed an element before it so index must change
    g->removeEdge(destinationEdge);

    g->addNode("source", std::any());
    g->addNode("destination", std::any());
    g->addEdge(sourceNode1, "source", sourceSection1, this->calculateCost(sourceSection1.polygon));
    g->addEdge("source", sourceNode2, sourceSection2, this->calculateCost(sourceSection2.polygon));
    g->addEdge(destNode1, "destination", destinationSection1, this->calculateCost(destinationSection1.polygon));
    g->addEdge("destination", destNode2, destinationSection2, this->calculateCost(destinationSection2.polygon));

    std::vector<PathInfo> path = g->dijkstra("source", "destination");
    return path;

}
