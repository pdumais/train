#ifndef MOCKVISIONSERVICE_H
#define MOCKVISIONSERVICE_H

#include <IVisionService.h>



class MockVisionService : public IVisionService
{
public:
    MockVisionService();

    // IVisionService interface
public:
    QVideoProbe *probe() override;
    CVObject locomotive() override;
    QVector<CVObject> wagons() override;
    std::vector<DetectedMarker> markers() override;
    void setTrackMask(QVector<QPolygon> tracks) override;
    void setRestrictLocomotiveDetectionToTracks(bool v) override;
    void enableAnnotationDetection(bool v) override;
};

#endif // MOCKVISIONSERVICE_H
