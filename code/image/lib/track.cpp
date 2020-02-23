#include "track.h"
#include "constants.h"
#include <QDebug>
#include <math.h>

Track::Track()
{
}

Track::Track(QString trackName)
{
    this->name = trackName;
}

QString Track::getName()
{
    return this->name;
}

void Track::setName(QString name)
{
    this->name = name;
}

QPolygon Track::getPolygon()
{
    return this->polygon;
}

void Track::setPolygon(QPolygon poly)
{
    this->polygon = poly;
}

bool Track::loops()
{
    if (this->polygon.size() < 10) return false;

    QPoint delta = this->polygon.first() - this->polygon.last();
    return delta.manhattanLength() < (DETECT_RECT*2);
}

// Returns the point on the track that is closest to the givven off-track point
QPoint Track::findClosestPoint(QPoint point)
{
    int lastDistance = 100000;
    QPoint ret = point;
    for (QPoint& p : this->polygon)
    {
        QPoint delta = p-point;
        int d = sqrt((delta.x()*delta.x())+(delta.y()*delta.y()));
        if (d < lastDistance)
        {
            lastDistance = d;
            ret = p;
        }
    }

    return ret;
}

QPolygon Track::extractSegment(QPoint first, QPoint last, bool reverse)
{
    QPolygon ret;
    first = this->findClosestPoint(first);
    last = this->findClosestPoint(last);

    int i1 = this->polygon.indexOf(first);
    int i2 = this->polygon.indexOf(last);

    while (i1!=i2)
    {
        ret.append(this->polygon[i1]);
        i1 += (reverse?-1:1);
        if (i1 >= this->polygon.size())
        {
            if (!this->loops())
            {
                qDebug() << "ERROR (" << this->name << "): Trying to rollover track segment but it's not a loop";
                return QPolygon();
            }
            i1 = 0;
        }
        if (i1 < 0)
        {
            if (!this->loops())
            {
                qDebug() << "ERROR (" << this->name << "): Trying to rollover track segment but it's not a loop";
                return QPolygon();
            }
            i1 = this->polygon.size()-1;
        }
    }
    ret.append(this->polygon[i1]);

    return ret;
}

QDataStream &operator<<(QDataStream &out, Track* t)
{
    out << t->getName() << t->getPolygon();
    return out;
}

QDataStream &operator>>(QDataStream &in, Track* &t)
{
    QString name;
    QPolygon poly;

    in >> name >> poly;
    t = new Track(name);
    t->setPolygon(poly);
    return in;
}

