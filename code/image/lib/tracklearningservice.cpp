#include "tracklearningservice.h"
#include <QDebug>
#include <QFont>
#include <QPoint>


TrackLearningService::TrackLearningService(TrainController *ctrl, VisionService* vision, QObject *parent) : QObject(parent)
{
    this->vision = vision;
    this->controller = ctrl;
    this->learning = false;
    this->path = nullptr;

    connect(this->vision, SIGNAL(locomotivePositionChanged(CVObject)), this, SLOT(on_locomotive_changed(CVObject)));
    connect(this->vision, SIGNAL(locomotiveLost()), this, SLOT(on_locomotive_lost()));

}

TrackLearningService::~TrackLearningService()
{
    if (this->path) delete this->path;
    this->path = nullptr;
}

void TrackLearningService::on_locomotive_lost()
{
    if (!this->learning) return;

    this->controller->setSpeed(0);
}

void TrackLearningService::on_locomotive_changed(CVObject obj)
{
    int x = obj.getCenter().x();
    int y = obj.getCenter().y();

    if (!this->learning) return;
    if (x == 0 || y == 0) return;

    // Check if image detection was halted because train was lost and resume
    if (!this->controller->getSpeed()) this->controller->setSpeed(100);

    if (!this->path)
    {
        this->path = new QPolygon();
        this->path->append(QPoint(x,y));
    }
    else
    {

        QPoint lastPos = this->path->last();
        QRect last(lastPos.x()-DETECT_RECT,lastPos.y()-DETECT_RECT,DETECT_RECT*2, DETECT_RECT*2);
        QPoint firstPos = this->path->first();
        QRect first(firstPos.x()-DETECT_RECT,firstPos.y()-DETECT_RECT,DETECT_RECT*2, DETECT_RECT*2);

        // If too close to last point, don't record it
        if (last.contains(x,y)) return;

        // If we have at least 10 points and we are back to first place, then we have looped.
        if (this->path->count() > 10 && first.contains(x,y))
        {
            this->stop();
            return;
        }


        this->path->append(QPoint(x,y));
        qDebug() << "Learning new position: " << QPoint(x,y);
    }

    QPainterPath p;
    p.addPolygon(*this->path);

    this->guiTrack->setPath(p);

}

void TrackLearningService::start(QString name, QGraphicsPathItem* guiTrack)
{
    if (this->path) delete this->path;
    this->path = nullptr;
    this->guiTrack = guiTrack;
    this->learning = true;
    this->name = name;

    this->vision->setRestrictLocomotiveDetectionToTracks(false);
    this->controller->setDirection(false);
    this->controller->setSpeed(100);
}

void TrackLearningService::stop()
{
    this->vision->setRestrictLocomotiveDetectionToTracks(true);
    this->controller->setSpeed(0);
    this->learning = false;
    this->guiTrack->setPath(QPainterPath());
    emit learningStopped(this->name);
}

QString TrackLearningService::getName()
{
    return this->name;
}

QPolygon* TrackLearningService::getLearnedPolygon()
{
    return this->path;
}
