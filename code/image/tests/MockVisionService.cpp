#include "MockVisionService.h"

MockVisionService::MockVisionService()
{

}


QVideoProbe *MockVisionService::probe()
{
    return nullptr;
}

CVObject MockVisionService::locomotive()
{
    return this->loco;
}

QVector<CVObject> MockVisionService::wagons()
{
    return this->wagonsList; 
}

std::vector<DetectedMarker> MockVisionService::markers()
{
    return std::vector<DetectedMarker>();
}

void MockVisionService::setTrackMask(QVector<QPolygon> tracks)
{
}

void MockVisionService::setRestrictLocomotiveDetectionToTracks(bool v)
{
}

void MockVisionService::enableAnnotationDetection(bool v)
{
}

QPixmap MockVisionService::getDebugImage(QString name)
{
    return QPixmap();
}

void MockVisionService::enableDebug(bool val)
{
}

QVector<QString> MockVisionService::getDebugNames()
{
    return QVector<QString>();
}
