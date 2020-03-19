#ifndef RAILROADLOGICSERVICE_H
#define RAILROADLOGICSERVICE_H

#include "constants.h"
#include "IDisplayService.h"
#include "ITrainController.h"
#include "IVisionService.h"
#include "AudioService.h"
#include "splitterannotation.h"
#include <QObject>
#include "Train.h"
#include "Railroad.h"

class ActionRunner;

class RailroadLogicService: public QObject
{
    Q_OBJECT
public:
    RailroadLogicService(ITrainController *ctrl, IVisionService* vision, IDisplayService* display, Configuration* configuration, AudioService* audioService);
    ~RailroadLogicService();

    void startTrainForward();
    void startTrainReverse();
    void stopTrain();
    void stopAll();
    void gotoWaypoint();

    void setWaypoint(QPoint p);
    void updateTracks();
    void updateAnnotations();

    void activateSplitter(SplitterAnnotation* sa, bool enabled);

    void updateTurnouts();

    ActionRunner* getActionRunner();

    QPoint getTrackWaypoint();

    Railroad* getRailroad();

    TrainPosition findClosestPoint(QPoint);


public slots:
    void on_frame_processed(CVObject obj, QVector<CVObject> wagons);
    void on_marker_found(DetectedMarker);
    void on_waypoint_set(QPoint);
    //void on_fingers_detected(QVector<QPoint>);

private:

    struct SplitterSearchNode
    {
        bool end;
        bool valid;
        SplitterAnnotation* sa;
        QString targetTrack;
        bool reverseThroughSplitter; // The direction to take in the splitter to reach the target
        bool reverseToReachSplitter; // if the track loops, we could travel in both direction to reach it
        QVector<SplitterSearchNode> children;
    };


    ActionRunner*  actionRunner;
    TrainPosition waypoint;
    ITrainController *controller;
    IVisionService* vision;
    IDisplayService* display;
    Configuration* configuration;
    AudioService* audioService;
    Train* train;
    Railroad* railroad;
    int resumeSpeed;

    void inRange(const QVector<Annotation*>& annotations, QVector<Annotation*>& targets, const CVObject& obj, int threshold);
    int getDistanceOnTrack(Track* track, QPoint source, QPoint destination, bool direction);
    void checkAnnotations();
};

#endif // RAILROADLOGICSERVICE_H
