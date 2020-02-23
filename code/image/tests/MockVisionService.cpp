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
    return CVObject();
}

QVector<CVObject> MockVisionService::wagons()
{
    return QVector<CVObject>();
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
