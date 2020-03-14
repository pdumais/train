#include "MockVisionService.h"

MockVisionService::MockVisionService()
{

}


QVideoProbe *MockVisionService::probe()
{
    return nullptr;
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

void MockVisionService::disableDebug()
{
}

void MockVisionService::enableDebug(QString)
{
}

QVector<QString> MockVisionService::getDebugNames()
{
    return QVector<QString>();
}
