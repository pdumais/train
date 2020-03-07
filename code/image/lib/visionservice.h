#pragma once

#include <QObject>
#include <QVideoProbe>
#include "cvobject.h"
#include "opencv/cv.h"
#include "opencv2/text.hpp"
#include "IVisionService.h"
#include "MatrixPool.h"

typedef std::vector<std::vector<cv::Point>> Contours;
    
class VisionService: public IVisionService
{
    Q_OBJECT
public:
    VisionService(int w, int h);
     ~VisionService() override;

    QVideoProbe* probe() override;

    CVObject locomotive() override;
    QVector<CVObject> wagons() override;
    std::vector<DetectedMarker> markers() override;

    void setTrackMask(QVector<QPolygon> tracks) override;
    void setRestrictLocomotiveDetectionToTracks(bool v) override;
    void enableAnnotationDetection(bool v) override;

    QPixmap getDebugImage(QString name) override;
    void enableDebug(bool val) override;
    QVector<QString> getDebugNames() override;

signals:
    void locomotivePositionChanged(CVObject);
    void frameProcessed();
    void locomotiveLost();
    void markerFound(DetectedMarker);

public slots:
    void processFrame(QVideoFrame);

private:
    struct DetectionSpecs
    {
        cv::Scalar minColor;
        cv::Scalar maxColor;
        int minThreshold;
        int maxThreshold;
        int areaSize;
        QString type;
    };

    struct DetectedObject 
    {
        cv::RotatedRect rect;
        int      detectionMiss;
    };

    bool annotationDetectionEnabled;
    int width;
    int height;
    bool restrictLocomotiveDetectionToTracks;
    QVideoProbe* videoProbe;
    QImage*     qtrackMask;
    cv::Mat     trackMask;
    int skipFrames;
    cv::Rect detectionRestriction;
    DetectedObject locomotive_position;
    std::vector<DetectedObject> wagons_positions;
    std::vector<DetectedMarker> markers_positions;
    DetectionSpecs locomotiveSpecs;
    DetectionSpecs locomotiveSpecsOffTracks;
    DetectionSpecs wagonSpecs;
    DetectionSpecs crossingSpecs;
    cv::Ptr<cv::text::OCRTesseract> ocr;

    std::vector<cv::RotatedRect> getObjects(cv::Mat* img, DetectionSpecs specs, Contours&);
    cv::RotatedRect getEnlargedRect(cv::RotatedRect r, int newW, int newH);
    QLineF getLine(cv::RotatedRect r);

    bool detectWagons(cv::Mat* mat);
    bool detectLocomotive(cv::Mat* mat, DetectionSpecs& specs);
    void detectMarkers(cv::Mat* mat);

    bool processDetectedWagons(std::vector<cv::RotatedRect>& wagons);
    bool processDetectedLocomotive(std::vector<cv::RotatedRect>& loco);

    MatrixPool matrixPool;
};
