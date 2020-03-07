#include "visionservice.h"
#include <QDebug>
#include <QPixmap>
#include <QLine>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include <opencv2/aruco.hpp>
#include "opencv/cv.h"
#include "displayservice.h"
#include "constants.h"
#include "PerformanceMonitor.h"

#define MATRIX_LOCO_MASK 0
#define MATRIX_WAGON_MASK 1
#define MATRIX_CROSSINGS_MASK 2
#define MATRIX_ALL_GRAY 3


#define SKIP_FRAMES 1
#define DETECTION_MISS_THRESHOLD 4
#define HSV(h,s,v) cv::Scalar(int(double(h)/2.0),int((double(s)/100.0)*255.0),int((double(v)/100.0)*255.0))

std::string whitelist = "AFHL";

VisionService::VisionService(int w, int h)
{
    this->width = w;
    this->height = h;
    this->annotationDetectionEnabled = true;
    this->qtrackMask = nullptr;
    // TODO: make this configurable
    this->detectionRestriction = cv::Rect(200,0,w-400,h);

    this->videoProbe = new QVideoProbe();
    connect(this->videoProbe, SIGNAL(videoFrameProbed(QVideoFrame)), this, SLOT(processFrame(QVideoFrame)));
    this->skipFrames = SKIP_FRAMES;

    this->locomotiveSpecs.minColor = HSV(340,0,0);    // pink
    this->locomotiveSpecs.maxColor = HSV(360,100,100);  // pink
    this->locomotiveSpecs.type = "locomotive";
    this->locomotiveSpecs.areaSize = DETECTION_SQUARE_SIZE;

    this->locomotiveSpecsOffTracks.minColor = HSV(340,20,60);    // pink
    this->locomotiveSpecsOffTracks.maxColor = HSV(360,100,100);  // pink
    this->locomotiveSpecsOffTracks.type = "locomotive";
    this->locomotiveSpecsOffTracks.areaSize = DETECTION_SQUARE_SIZE;

    this->wagonSpecs.minColor = HSV(60,40,40);
    this->wagonSpecs.maxColor = HSV(160,100,100);
    this->wagonSpecs.type = "wagon";
    this->wagonSpecs.areaSize = DETECTION_SQUARE_SIZE;

    this->crossingSpecs.minColor = HSV(40,50,20);
    this->crossingSpecs.maxColor = HSV(50,90,60);
    this->crossingSpecs.type = "crossing";
    this->crossingSpecs.areaSize = ANNOTATION_DETECTION_SQUARE_SIZE;

    this->locomotive_position.detectionMiss = 0;

    this->restrictLocomotiveDetectionToTracks = true;

}


VisionService::~VisionService()
{
    delete this->videoProbe;
}

void VisionService::enableAnnotationDetection(bool v)
{
    this->annotationDetectionEnabled = v;
}

void VisionService::setRestrictLocomotiveDetectionToTracks(bool v)
{
    this->restrictLocomotiveDetectionToTracks = v;
}

QVideoProbe* VisionService::probe()
{
    return this->videoProbe;
}

QLineF VisionService::getLine(cv::RotatedRect r)
{
    cv::Point2f points[4];
    cv::Point2f c;
    r.points(points);
    c = r.center;

    cv::Point2f pd1 = points[1] - points[0];
    cv::Point2f pd2 = points[3] - points[0];
    double side1 = sqrt((pd1.x*pd1.x)+(pd1.y*pd1.y));
    double side2 = sqrt((pd2.x*pd2.x)+(pd2.y*pd2.y));

    cv::Point2f lp1,lp2,lpc;
    lp1 = points[0];

    if (side1 > side2)
    {
        lp2 = points[1];
        lpc = lp1+(pd1/2); 
    }
    else
    {
        lp2 = points[3];
        lpc = lp1+(pd2/2); 
    }

    cv::Point2f lpcdelta = c - lpc;
    // translate lp1 and lp2 so that the line they form goes through the rect's center point
    lp1 += lpcdelta;
    lp2 += lpcdelta;

    QLineF line(QPointF(lp1.x, lp1.y),QPointF(lp2.x,lp2.y));

    return line;

}

std::vector<DetectedMarker> VisionService::markers()
{
    return this->markers_positions;
}

//TODO: dont recalc this everytime. We should cache it
CVObject VisionService::locomotive()
{
    cv::RotatedRect r = this->locomotive_position.rect;
    QPolygon poly;
    cv::Point2f points[4];
    r.points(points);
    for (int i = 0; i < 4; i++)
    {
        poly << QPoint(points[i].x,points[i].y);
    }

    cv::Point2f c;
    c = r.center;
    auto line = this->getLine(r);
    CVObject cvo;
    cvo.setCenter(QPoint(c.x,c.y));
    cvo.setPolygon(poly);
    cvo.setLine(line.toLine());

    return cvo;
}

//TODO: dont recalc this everytime. We should cache it
QVector<CVObject> VisionService::wagons()
{
    QVector<CVObject> ret;

    for (DetectedObject o : this->wagons_positions)
    {
        QPolygon poly;
        cv::Point2f points[4];
        o.rect.points(points);
        for (int i = 0; i < 4; i++)
        {
            poly << QPoint(points[i].x,points[i].y);
        }

        cv::Point2f c;
        c = o.rect.center;
        QLineF line = this->getLine(o.rect);
        CVObject cvo;
        cvo.setCenter(QPoint(c.x,c.y));
        cvo.setPolygon(poly);
        cvo.setLine(line.toLine());
        ret.push_back(cvo);
    }

    return ret;
}

std::vector<cv::RotatedRect> VisionService::getObjects(cv::Mat* img, DetectionSpecs specs, Contours& contours)
{
    std::vector<cv::RotatedRect> ret;

    cv::Mat* mat = this->matrixPool.getMatrix("object_detect");
    cv::Mat tmp;
    cv::inRange(*img, specs.minColor, specs.maxColor, *mat);
    cv::erode(*mat,tmp,cv::Mat());
    cv::dilate(tmp,*mat,cv::Mat());

    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(*mat, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_NONE);

    if (contours.size() < 1) return ret;

    int contourIndex = 0;
    while (contourIndex != -1)
    {
        double childArea = 0;

        int nextIndex = hierarchy[contourIndex][0];
        int childIndex = hierarchy[contourIndex][2];
        auto contour = contours[contourIndex];
        contourIndex = nextIndex;

        double area = cv::contourArea(contour);
        if (area < specs.areaSize) continue;

        bool bigChild = false;
        while (childIndex != -1)
        {
            childArea = cv::contourArea(contours[childIndex]);
            childIndex = hierarchy[childIndex][0]; // get next sibling
            if (childArea > (area/10))
            {
                bigChild = true;
                break;
            }
        }
        // If the contour has an inner contour that is bigger than 10% of the whole shape, then ignore it.
        if (bigChild) continue;


        //TODO: we could also check for the aspectRatio to make sure it is something like a rectangle
        cv::RotatedRect r = cv::minAreaRect(contour);
        ret.push_back(r);
    }
    return ret;
}

// This is for getting the wagons mask using contour. It is slower than using getRectWagonMask but it's more precise
void VisionService::getContourWagonMask(DetectionSpecs specs, Contours& contours, cv::Mat& mask)
{
    Contours list;
    // Create mask using contours. This is more precise but sometimes
    // bleeds into the shape, flooding the letter
    for (auto& it : contours)
    {
        double epsilon = 0.1*cv::arcLength(it,true);
        std::vector<cv::Point> curve;
        cv::approxPolyDP(it,curve,epsilon,true);
        double area = cv::contourArea(curve);
        if (area < specs.areaSize) continue;
        list.push_back(it);
    }
    cv::drawContours(mask,list, -1, cv::Scalar(255),-1);
}

// This is for getting the wagons mask using rotated rect. It is faster than using getContourWagonMask but it's sloppier
void VisionService::getRectWagonMask(std::vector<cv::RotatedRect>& wagons, cv::Mat& mask)
{
    // Create mask using rotated rectangles
    for (cv::RotatedRect r : wagons)
    {
        cv::Point2f p2f[4];
        cv::Point p[4];
        r.points(p2f);
        for (int i=0; i<4; i++) p[i] = p[i] = p2f[i];
        cv::fillConvexPoly(mask,p,4,cv::Scalar(255));
    }
}

void VisionService::identifyWagons(std::vector<cv::RotatedRect>& wagons, cv::Mat& grayImage)
{
    cv::Mat maskedWagon(this->height, this->width, CV_8UC1);
    cv::Mat mask = cv::Mat(this->height, this->width, CV_8UC1);
    mask.setTo(cv::Scalar(0));
    getRectWagonMask(wagons,mask);
    //getContourWagonMask(this->wagonSpecs, contours,mask);

    // Using a negative mask, set background white
    cv::Mat negativeMask = cv::Mat(this->height, this->width, CV_8UC1);
    negativeMask.setTo(cv::Scalar(255));
    cv::Mat tmp = cv::Mat(this->height, this->width, CV_8UC1);
    cv::bitwise_not(mask,negativeMask);
    cv::bitwise_or(grayImage,negativeMask,tmp);
    cv::threshold(tmp, maskedWagon, 70, 255, cv::THRESH_BINARY);


    std::string wagonLettersFound;
    for (auto& it : wagons)
    {
        cv::Rect r = it.boundingRect();
        cv::Mat label = maskedWagon(r);
        label = label.clone();
        cv::Mat rotatedLabel(100,100,CV_8UC1);
        cv::Point2f center(r.width/2,r.height/2);

        // if the width is larger than the height, it means that the rectangle is lying on its side.
        // So we'll add another 90 degrees to the angle
        float angle = it.angle;
        if (it.size.width > it.size.height) angle-=90;

        cv::Mat rotation = cv::getRotationMatrix2D(center, angle, 1);
        warpAffine(label, rotatedLabel, rotation, rotatedLabel.size(), cv::INTER_CUBIC,cv::BORDER_CONSTANT, cv::Scalar(255));

        std::string str;
        ocr->run(rotatedLabel,str);
        char letter = 0;
        for (char c : whitelist)
        {
            if (str.find(c) != std::string::npos)
            {
                letter = c;
                break;
            }
        }
        if (!letter)
        {
            cv::Mat rotation = cv::getRotationMatrix2D(center, angle+180, 1);
            warpAffine(label, rotatedLabel, rotation, rotatedLabel.size(), cv::INTER_CUBIC,cv::BORDER_CONSTANT, cv::Scalar(255));
            ocr->run(rotatedLabel,str);
            for (char c : whitelist)
            {
                if (str.find(c) != std::string::npos)
                {
                    letter = c;
                    break;
                }
            }
        }
        if (letter) wagonLettersFound += letter;
        //DEBUGIMG8(rotatedLabel);
    }

    qDebug() << wagonLettersFound.c_str();
}

cv::RotatedRect VisionService::getEnlargedRect(cv::RotatedRect r, int newW, int newH)
{
    cv::RotatedRect ret;

    cv::Point2f center = r.center;
    cv::Size2f size = r.size;
    float angle = r.angle;

    if (size.width > size.height)
    {
        size.width = newW;
        size.height = newH;
    }
    else
    {
        size.width = newH;
        size.height = newW;
    }

    return cv::RotatedRect(center,size,angle);

}

bool VisionService::detectWagons(cv::Mat* mat)
{
    Contours contours;

    this->matrixPool.setContext("wagons");
    std::vector<cv::RotatedRect> wagons = this->getObjects(mat, this->wagonSpecs, contours);
    //this->identifyWagons(wagons, grayImage);

    return this->processDetectedWagons(wagons);
}

bool VisionService::processDetectedWagons(std::vector<cv::RotatedRect>& wagons)
{
    bool atLeastOneDetected = false;

    this->wagons_positions.clear();
    for (auto &it : wagons)
    {
        DetectedObject obj;
        obj.rect = this->getEnlargedRect(it,WAGON_WIDTH,WAGON_HEIGHT);
        this->wagons_positions.push_back(obj);
        atLeastOneDetected = true;
    }

    return atLeastOneDetected;
}

bool VisionService::detectLocomotive(cv::Mat* mat, DetectionSpecs& specs)
{
    Contours contours;

    this->matrixPool.setContext("locomotive");
    std::vector<cv::RotatedRect> points = this->getObjects(mat, specs, contours);

    return this->processDetectedLocomotive(points);
}

bool VisionService::processDetectedLocomotive(std::vector<cv::RotatedRect>& loco)
{
    if (loco.size() != 1)
    {
        // We only emit once, so when detectionMiss matches, not when it is greater than
        this->locomotive_position.detectionMiss++;
        if (this->locomotive_position.detectionMiss == DETECTION_MISS_THRESHOLD)
        {
            emit this->locomotiveLost();
        }

        return false;
    }

    this->locomotive_position.detectionMiss = 0;
    this->locomotive_position.rect = this->getEnlargedRect(loco[0],LOCO_WIDTH,LOCO_HEIGHT);

    return true;

}


void VisionService::detectMarkers(cv::Mat* mat)
{
    Contours contours;
    this->matrixPool.setContext("markers");

    std::vector<cv::RotatedRect> crossings = this->getObjects(mat, this->crossingSpecs, contours);
    for (auto c : crossings)
    {
        DetectedMarker dm;
        dm.pos = QPoint(c.center.x, c.center.y);
        dm.code = MARKER_TYPE_CROSSING;
        emit markerFound(dm);
    }

}

void VisionService::processFrame(QVideoFrame frame)
{
    cv::Point2f v2f[4];
    cv::Point v[4];
    cv::Mat tmp, tmp2, tmp3;

    PerformanceMonitor::tic("VisionService::processFrame");

    this->matrixPool.reset();
    this->matrixPool.setContext("process_frame");

    // The frame comming from the camera are RGB32. Other cams could return something else so we'd need to adjust this.
    frame.map(QAbstractVideoBuffer::ReadOnly);


    cv::Mat frameMat(frame.height(), frame.width(), CV_8UC4, frame.bits());
    cv::Mat* grayImage = this->matrixPool.getMatrix("gray_all");
    cv::Mat* hsvImage = this->matrixPool.getMatrix("hsv_all");
    grayImage->create(frame.height(), frame.width(), CV_8UC1);
    hsvImage->create(frame.height(), frame.width(), CV_8UC4);
    cv::cvtColor(frameMat, *grayImage, cv::COLOR_BGR2GRAY);
    cv::cvtColor(frameMat, *hsvImage, cv::COLOR_BGR2HSV);


    // Get the track-masked image
    cv::Mat* masked = this->matrixPool.getMatrix("track_masked");
    frameMat.copyTo(tmp, this->trackMask);
    cv::cvtColor(tmp, *masked, cv::COLOR_BGR2HSV);
    // At this point we have 2 images: "hsvImage" which is the whole image. and "masked" which is restricted on the map


    bool useDottedDetection = false;
    bool locoDetected = false;

    if (useDottedDetection)
    {
        locoDetected = this->detectDottedLabels(frame.width(), frame.height(), grayImage);

    }
    else
    {
        if (this->annotationDetectionEnabled)
        {
            this->detectMarkers(masked);
        }

        // Detect wagons on the masked image
        this->detectWagons(masked);

        // Get locomotive
        if (this->restrictLocomotiveDetectionToTracks)
        {
            locoDetected = this->detectLocomotive(masked, this->locomotiveSpecs);
        }

    }

    if (!this->restrictLocomotiveDetectionToTracks)
    {
        locoDetected = this->detectLocomotive(hsvImage, this->locomotiveSpecsOffTracks);
    }
    if (locoDetected)
    {
        emit locomotivePositionChanged(this->locomotive());
    }

    emit frameProcessed();

    PerformanceMonitor::toc("VisionService::processFrame");
}


bool VisionService::detectDottedLabels(int w, int h, cv::Mat* grayImage)
{
    //////////////////////////////////
    // THIS IS NOT USED
    //////////////////////////////////
    Contours list;
    std::vector<cv::RotatedRect> locos;
    std::vector<cv::RotatedRect> wagons;
    Contours contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::Mat* maskedCandidates = this->matrixPool.getMatrix("masked_candidates");
    cv::Mat* contoursMat = this->matrixPool.getMatrix("contours");
    contoursMat->create(h, w, CV_8UC1);
    cv::Mat tmp, tmp2, tmp3;

    cv::Mat* thresh = this->matrixPool.getMatrix("threshold");
    cv::threshold(*grayImage, *thresh, 60, 255, cv::THRESH_BINARY);

    //Get the adaptive threshold image
    cv::Mat* adaptive = this->matrixPool.getMatrix("adaptive");
    cv::adaptiveThreshold(*grayImage, *adaptive, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 5,3);
    adaptive->copyTo(tmp, this->trackMask);


    cv::Mat* adaptiveeroded = this->matrixPool.getMatrix("adaptive_eroded");
    cv::erode(tmp, *adaptiveeroded, cv::Mat());

    cv::findContours(*adaptiveeroded, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
    cv::Mat mc(h,w,CV_8UC1);
    mc.setTo(255);
    for (int i =0; i < contours.size(); i++)
    {
        auto it = contours[i];
        cv::RotatedRect rr = cv::minAreaRect(it);
        cv::Rect r = cv::boundingRect(it);
        double area = rr.size.area();
        //float ratio = rr.size.width/rr.size.height;
        if (area < 200 || area >700) continue;
        if (r. width > 100 || r.height > 100) continue;
        if (r. width < 10 || r.height < 10) continue;
        //if (ratio > 4.0 || ratio < 0.25) continue;

        // The shape must have between 1 and 3 dots in it. So The contour must have between 1-3 children
        int childrenCount = 0;
        int childIndex = hierarchy[i][2];
        while (childIndex != -1)
        {
            auto it2 = contours[childIndex];
            //cv::Rect rc = cv::boundingRect(it2);
            cv::RotatedRect rr2 = cv::minAreaRect(it2);

            childIndex =  hierarchy[childIndex][0];

            if (rr2.size.area() < 10) continue;

            childrenCount++;
        }
        if (childrenCount > 5) continue;
        //if (childrenCount <1 || childrenCount > 3) continue;

        //if (childrenCount == 1 ) locos.push_back(rr);
        //else if (childrenCount == 2 ) wagons.push_back(rr);

        //qDebug() << childrenCount;


        cv::Mat source = (*grayImage)(r);
        source = source.clone();
        cv::Mat rotated(r.height,r.width,CV_8UC1);
        cv::Point2f center(r.width/2,r.height/2);
        float angle = rr.angle+90;
        if (rr.size.width > rr.size.height) angle-=90;

        cv::Mat rotation = cv::getRotationMatrix2D(center, angle, 1);
        warpAffine(source, rotated, rotation, rotated.size(), cv::INTER_CUBIC,cv::BORDER_CONSTANT, cv::Scalar(255));

        rotated.copyTo(mc.colRange(r.x,r.x+r.width).rowRange(r.y,r.y+r.height));
//        rotated.copyTo(maskedCandidates->rowRange(r.x,r.x+rr.size.width).colRange(r.y,r.y+rr.size.height));
        /*
        cv::Point2f p2f[4];
        cv::Point p[4];
        rr.points(p2f);
        for (int i=0; i<4; i++) p[i] = p[i] = p2f[i];
        cv::fillConvexPoly(*contoursMat, p, 4, cv::Scalar(255));*/

    }
    mc.copyTo(*maskedCandidates);
    //grayImage->copyTo(tmp2, *contoursMat);
    //cv::threshold(tmp2, *maskedCandidates, 100, 255, cv::THRESH_BINARY);

    qDebug() << "";
    //adaptive->copyTo(*maskedCandidates, *contoursMat);

    this->processDetectedWagons(wagons);
    bool locoDetected = false;
    if (!this->restrictLocomotiveDetectionToTracks)
    {
        locoDetected = this->processDetectedLocomotive(locos);
    }

    return locoDetected;
}


void VisionService::setTrackMask(QVector<QPolygon> tracks)
{
    if (this->qtrackMask) delete this->qtrackMask;
    this->qtrackMask = new QImage(this->width, this->height, QImage::Format_Grayscale8);
    QPainter p(this->qtrackMask);
    QPen pen(Qt::white);
    p.fillRect(this->qtrackMask->rect(),Qt::black);
    pen.setWidth(TRACK_WIDTH);
    p.setPen(pen);
        
    /// manually draw each segment. Becuase drawPolygon would close the shape
    for (QPolygon& poly : tracks)
    {
        if (poly.isEmpty()) continue;
        QPoint first = poly.takeFirst();
        QPainterPath path(first);
        for (auto point : poly)
        {
            path.lineTo(point);
        }
        p.drawPath(path);
    }

    cv::Mat tmp = cv::Mat(this->qtrackMask->height(), this->qtrackMask->width(), CV_8UC1, this->qtrackMask->bits());
    cv::threshold(tmp, this->trackMask, 200 , 255,0);
}


QPixmap VisionService::getDebugImage(QString name)
{
     return QPixmap::fromImage(this->matrixPool.dumpMatrix(name));
}

void VisionService::enableDebug(bool val)
{
    this->matrixPool.setDebug(val);
}

QVector<QString> VisionService::getDebugNames()
{
    return this->matrixPool.getNames();
}
