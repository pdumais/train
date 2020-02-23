#include "Train.h"

#include <QRegion>
#include <QDebug>
#include "constants.h"
#include "math.h"

Train::Train(CVObject loco, QObject *parent) : QObject(parent)
{
    this->locomotive = new TrainPart();
    this->locomotive->object = loco.getPolygon();
    this->locomotive->center = loco.getCenter();
    this->locomotive->line = loco.getLine();
    this->locomotive->next = nullptr;
    this->locomotive->previous = nullptr;
}

Train::~Train()
{
    delete this->locomotive;
    for (auto tp : this->wagons)
    {
        delete tp;
    }
}


void Train::addWagon(CVObject w)
{
    TrainPart* tp = new TrainPart();
    tp->object = w.getPolygon();
    tp->center = w.getCenter();
    tp->line = w.getLine();

    this->wagons.append(tp);
    this->sortTrainParts(tp);

}

QPoint Train::getPosition()
{
    return this->locomotive->center;
}

bool Train::inRange(QPoint p, int radius) const
{
    QRegion target(QRect(p.x()-radius,p.y()-radius, radius*2, radius*2), QRegion::RegionType::Ellipse);

    QRegion r(this->locomotive->object);
    if (r.intersects(target)) return true;

    for (auto it : this->getLinkedWagons())
    {
        QRegion r(it);
        if (r.intersects(target)) return true;
    }

    return false;
}

QVector<Annotation*> Train::inRange(const QVector<Annotation*>& annotations) const
{
    QVector<Annotation*> ret;
    for (Annotation* ca : annotations)
    {
        if (this->inRange(ca->getPosition(), ca->getRadius()))
        {
            ret.append(ca);
        }
    }

    return ret;
}

// Returns:
//      -1 if l1's end is tied to l2
//      0 if they are not tied
//      1 if l1's start is tied to l2
int Train::areLinked(QLine l1, QLine l2)
{
    if (HYPOTHENUS((l1.p1()-l2.p1())) < MAXIMUM_WAGON_CONNECTION_SIZE) return 1;
    if (HYPOTHENUS((l1.p1()-l2.p2())) < MAXIMUM_WAGON_CONNECTION_SIZE) return 1;
    if (HYPOTHENUS((l1.p2()-l2.p1())) < MAXIMUM_WAGON_CONNECTION_SIZE) return -1;
    if (HYPOTHENUS((l1.p2()-l2.p2())) < MAXIMUM_WAGON_CONNECTION_SIZE) return -1;

    return 0;
}

void Train::sortTrainParts(TrainPart* w)
{   
    w->previous = 0;
    w->next = 0;

    int r = this->areLinked(w->line, this->locomotive->line);
    if (r == 1)
    {
        this->locomotive->next = w;
        w->previous = this->locomotive;
    }
    else if (r == -1)
    {
        this->locomotive->previous = w;
        w->next = this->locomotive;
    }

    for (TrainPart* tp : this->wagons)
    {
        if (tp == w) continue;
        int r = this->areLinked(w->line, tp->line);
        if (!r) continue;

        if (r == 1)
        {
            tp->next = w;
            w->previous = tp;
        }
        else if (r == -1)
        {
            tp->previous= w;
            w->next = tp;
        }
    }
}

QPolygon Train::getLocomotive()
{
    return this->locomotive->object;
}

QVector<QPolygon> Train::getLinkedWagons() const
{
    QVector<QPolygon> ret;

    TrainPart* tp = this->locomotive->next;
    while (tp)
    {
        ret.append(tp->object);
        tp = tp->next;
    }

    tp = this->locomotive->previous;
    while (tp)
    {
        ret.append(tp->object);
        tp = tp->previous;
    }

    return ret;
}

QVector<QPolygon> Train::getUnlinkedWagons() const
{
    // TODO: need to optimize this
    QVector<QPolygon> ret;
    QVector<QPolygon> linked = this->getLinkedWagons();
    int count = this->wagons.size() - linked.size();
    for (int i = 0; (i < this->wagons.size() && count > 0); i++)
    {
        auto obj = this->wagons[i]->object;
        if (linked.contains(obj)) continue;

        count--;
        ret.append(obj);
    }

    return ret;
}
