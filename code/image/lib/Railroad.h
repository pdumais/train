#pragma once

#include <QVector>
#include "splitterannotation.h"
#include "track.h"
#include "Graph.h"

class Railroad
{
public:
    Railroad();
    ~Railroad();

    void setTracks(QVector<Track*> val);
    QVector<SplitterAnnotation*> getTurnouts();

    Graph* getGraph();

    std::vector<PathInfo> findShortestPath(QPoint source, QPoint dest);

private:
    QVector<SplitterAnnotation*> turnouts;
    QVector<Track*> tracks;
    Graph *tracksGraph;

    void findTurnouts();
    void buildGraph();

    int getClosestTurnout(SplitterAnnotation* current, QString track, bool reverse, SplitterAnnotation*& splitter, QPolygon& section);

    int calculateCost(const QPolygon& poly);
};
