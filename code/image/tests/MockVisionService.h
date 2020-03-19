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
    void setTrackMask(QVector<QPolygon> tracks) override;
    void setRestrictLocomotiveDetectionToTracks(bool v) override;
    void enableAnnotationDetection(bool v) override;
    void disableDebug() override;
    void enableDebug(QString) override;
    QVector<QString> getDebugNames() override;
    CVObject loco;
};

#endif // MOCKVISIONSERVICE_H
