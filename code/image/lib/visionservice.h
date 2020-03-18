#pragma once

#include <QObject>
#include <QTime>
#include <QVideoProbe>
#include "cvobject.h"
#include "opencv/cv.h"
#include "opencv2/text.hpp"
#include "IVisionService.h"
#include "MatrixPool.h"
#include "FrameProcessingWorker.h"
#include <opencv2/video.hpp>

typedef std::vector<cv::Point> Contour;
typedef std::vector<Contour> Contours;
    
class VisionService: public IVisionService
{
    Q_OBJECT
public:
    VisionService(int w, int h);
     ~VisionService() override;

    QVideoProbe* probe() override;

    // TODO: need to make the following methods thread-safe
    void setTrackMask(QVector<QPolygon> tracks) override;
    void setRestrictLocomotiveDetectionToTracks(bool v) override;
    void enableAnnotationDetection(bool v) override;

    void enableDebug(QString name) override;
    void disableDebug() override;
    QVector<QString> getDebugNames() override;

signals:
    void frameProcessed(CVObject, QVector<CVObject>);

    void locomotiveLost();
    void markerFound(DetectedMarker);
    void fingersDetected(QVector<QPoint>);
    void debugMatrixReady(QImage);

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

    std::vector<cv::RotatedRect> getObjects(int matrixPoolIndex, std::shared_ptr<cv::Mat> img, DetectionSpecs specs, Contours&);
    cv::RotatedRect getEnlargedRect(cv::RotatedRect r, int newW, int newH);
    QLineF getLine(cv::RotatedRect r);
    cv::Mat generateCrossingTemplate(int w, int h);

    bool detectWagons(std::shared_ptr<cv::Mat> mat);
    bool detectLocomotive(std::shared_ptr<cv::Mat> mat, DetectionSpecs& specs);
    void detectMarkers(std::shared_ptr<cv::Mat> mat, std::shared_ptr<cv::Mat> adaptive);

    bool processDetectedWagons(std::vector<cv::RotatedRect>& wagons);
    bool processDetectedLocomotive(std::vector<cv::RotatedRect>& loco);

    MatrixPool* matrixPool;
    MatrixPool* trackMatrixPool;

    FrameProcessingWorker* trackWorker;

    void workOnTrackDetection(std::shared_ptr<cv::Mat> original, std::shared_ptr<cv::Mat> hsv, std::shared_ptr<cv::Mat> gray, std::shared_ptr<cv::Mat> adaptive);

    CVObject locomotive();
    QVector<CVObject> wagons();
    std::vector<DetectedMarker> markers();

};
