#ifndef TRACK_H
#define TRACK_H

#include <QString>
#include <QMetaType>
#include <QPolygon>


class TrackSection
{
public:
    QPolygon polygon;
    QString trackName;
};

class Track
{
private:
    QString name;
    QPolygon polygon;

public:
    Track();
    Track(QString trackName);

    QString getName();
    void setName(QString name);
    QPolygon getPolygon();
    void setPolygon(QPolygon);

    bool loops();
    int findClosestIndex(QPoint p);
    QPoint findClosestPoint(QPoint p);
    QPolygon extractSegment(QPoint first, QPoint last, bool reverse);

};

Q_DECLARE_METATYPE(Track*);

QDataStream &operator<<(QDataStream &out, Track* t);
QDataStream &operator>>(QDataStream &in, Track* &t);


#endif // TRACK_H
