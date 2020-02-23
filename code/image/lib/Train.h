#ifndef TRAIN_H
#define TRAIN_H

#include <QObject>
#include <QPoint>
#include <QPolygon>
#include "track.h"
#include "annotation.h"

struct TrainPosition
{
    Track* track;
    QPoint point;
};

class Train : public QObject
{
    Q_OBJECT
public:
    explicit Train(QPolygon locomotive, QPoint center, QObject *parent = nullptr);

    void addWagon(QPolygon w, QPoint center);

    QPoint getPosition();
    bool inRange(QPoint p, int radius) const;
    QVector<Annotation*> inRange(const QVector<Annotation*>& annotations) const;

signals:

public slots:

private:
    struct TrainPart
    {
        QPolygon object;
        QPoint center;
    };

    TrainPart locomotive;
    QVector<TrainPart> wagons;

};

#endif // TRAIN_H
