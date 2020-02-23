#ifndef TRACKLEARNER_H
#define TRACKLEARNER_H

#include <QObject>
#include <QGraphicsPathItem>
#include <QPolygon>
#include "traincontroller.h"
#include "visionservice.h"
#include "cvobject.h"

class TrackLearningService : public QObject
{
    Q_OBJECT
public:
    explicit TrackLearningService(TrainController* controller, VisionService* vision, QObject *parent = nullptr);
    virtual ~TrackLearningService();

    void start(QString name, QGraphicsPathItem* guiTrack);
    void stop();
    QString getName();
    QPolygon* getLearnedPolygon();

signals:
    void    learningStopped(QString trackName);

public slots:
    void on_locomotive_changed(CVObject);
    void on_locomotive_lost();

private:
    TrainController* controller;
    VisionService* vision;
    QString name;
    bool learning;
    QPolygon *path;
    QGraphicsPathItem* guiTrack;
};

#endif // TRACKLEARNER_H
