#ifndef IVISIONSERVICE_H
#define IVISIONSERVICE_H
#include <QObject>
#include <QVideoProbe>
#include "cvobject.h"
#include "opencv/cv.h"
#include "opencv2/text.hpp"

struct DetectedMarker
{
    QPoint   pos;
    int      code;
};


class IVisionService: public QObject
{
    Q_OBJECT
public:
    IVisionService() {}
    virtual ~IVisionService() {}

    virtual QVideoProbe* probe() =0;

    virtual void setTrackMask(QVector<QPolygon> tracks) =0;
    virtual void setRestrictLocomotiveDetectionToTracks(bool v) =0;
    virtual void enableAnnotationDetection(bool v) =0;
    virtual void enableDebug(QString name) = 0;
    virtual void disableDebug() = 0;
    virtual QVector<QString> getDebugNames() = 0;
signals:
    void locomotivePositionChanged(CVObject);
    void locomotiveLost();
    void markerFound(DetectedMarker);

};

Q_DECLARE_METATYPE(DetectedMarker);

#endif // IVISIONSERVICE_H
