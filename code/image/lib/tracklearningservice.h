#ifndef TRACKLEARNER_H
#define TRACKLEARNER_H

#include <QObject>
#include <QGraphicsPathItem>
#include <QPolygon>
#include "traincontroller.h"
#include "visionservice.h"
#include "configuration.h"
#include "cvobject.h"

class TrackLearningService : public QObject
{
    Q_OBJECT
public:
    explicit TrackLearningService(TrainController* controller, VisionService* vision, Configuration* conf, QObject* listener, QObject *parent = nullptr);
    virtual ~TrackLearningService();

    void start(QString name);
    void stop();
    QString getName();
    QPolygon* getLearnedPolygon();

signals:
    void    learningStopped(QString trackName);
    void    trackUpdated(QPainterPath);

public slots:
    void on_frame_processed(CVObject, QVector<CVObject>);
    void on_locomotive_lost();

private:
    TrainController* controller;
    VisionService* vision;
    Configuration *configuration;
    QString name;
    bool learning;
    QPolygon *path;
};

#endif // TRACKLEARNER_H
