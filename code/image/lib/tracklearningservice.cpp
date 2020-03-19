#include "tracklearningservice.h"
#include <QDebug>
#include <QFont>
#include <QPoint>


TrackLearningService::TrackLearningService(TrainController *ctrl, VisionService* vision, Configuration *conf, QObject* listener, QObject *parent) : QObject(parent)
{
    this->configuration = conf;
    this->vision = vision;
    this->controller = ctrl;
    this->learning = false;
    this->path = nullptr;

    //TODO: how to make display receive that event without having a dependency on tracklearningservice?
    connect(this, SIGNAL(trackUpdated(QPainterPath)), listener, SLOT(on_learning_track_updated(QPainterPath)));
    connect(this->vision, SIGNAL(frameProcessed(CVObject, QVector<CVObject>)), this, SLOT(on_frame_processed(CVObject, QVector<CVObject>)));
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

void TrackLearningService::on_frame_processed(CVObject obj, QVector<CVObject>)
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
    
        QPoint newPoint(x,y);
        // Check if we've reached another track
        for (auto t : this->configuration->getTracks())
        {
            if ((t->findClosestPoint(newPoint)-newPoint).manhattanLength() < DETECT_RECT)
            {
                this->stop();
                this->path->append(newPoint);
                return;
            }
        }

        this->path->append(newPoint);
        qDebug() << "Learning new position: " << newPoint;
    }

    QPainterPath p;
    p.addPolygon(*this->path);

    emit trackUpdated(p);

}

void TrackLearningService::start(QString name)
{
    if (this->path) delete this->path;
    this->path = nullptr;
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
    emit trackUpdated(QPainterPath());
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
