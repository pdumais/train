#ifndef MOCKVISIONSERVICE_H
#define MOCKVISIONSERVICE_H

#include <IVisionService.h>
#include <QPixmap>


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
    QPixmap getDebugImage(QString name) override;
    void enableDebug(bool val) override;
    QVector<QString> getDebugNames() override;
    CVObject loco;
    QVector<CVObject> wagonsList;
};

#endif // MOCKVISIONSERVICE_H
