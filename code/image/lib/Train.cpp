#include "Train.h"

#include <QRegion>

Train::Train(QPolygon locomotive, QPoint center, QObject *parent) : QObject(parent)
{
    this->locomotive.object = locomotive;
    this->locomotive.center = center;
}

void Train::addWagon(QPolygon w, QPoint center)
{
    TrainPart tp;
    tp.object = w;
    tp.center = center;
    this->wagons.append(tp);
}

QPoint Train::getPosition()
{
    return this->locomotive.center;
}

bool Train::inRange(QPoint p, int radius) const
{
    QRegion target(QRect(p.x()-radius,p.y()-radius, radius*2, radius*2), QRegion::RegionType::Ellipse);

    QRegion r(this->locomotive.object);
    if (r.intersects(target)) return true;

    for (auto it : this->wagons)
    {
        QRegion r(it.object);
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
