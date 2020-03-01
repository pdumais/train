#ifndef TRAIN_H
#define TRAIN_H

#include <QObject>
#include <QPoint>
#include <QPolygon>
#include <QLine>
#include "track.h"
#include "annotation.h"
#include "cvobject.h"

struct TrainPosition
{
    Track* track;
    QPoint point;
};

class Train : public QObject
{
    Q_OBJECT
public:
    explicit Train(CVObject loco, QObject *parent = nullptr);
    ~Train();

    void addWagon(CVObject obj);
    
    QPolygon getLocomotive();
    QVector<QPolygon> getLinkedWagons() const;
    QVector<QPolygon> getUnlinkedWagons() const;

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
        QLine line;
        TrainPart *next;
        TrainPart *previous;
    };

    TrainPart* locomotive;
    QVector<TrainPart*> wagons;

    int areLinked(QLine l1, QLine l2);
    void sortTrainParts(TrainPart* tp);
};

#endif // TRAIN_H
