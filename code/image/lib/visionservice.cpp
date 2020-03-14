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
#include <QMap>

//#define MATRIX_LOCO_MASK 0
//#define MATRIX_WAGON_MASK 1
//#define MATRIX_CROSSINGS_MASK 2
//#define MATRIX_ALL_GRAY 3

#define MATRIX_ORIGINAL             0
#define MATRIX_GRAY_ALL             1
#define MATRIX_HSV_ALL              2
#define MATRIX_ADAPTIVE             3

#define MATRIX_DIFFERENCE           0

#define MATRIX_TRACK_MASKED         0
#define MATRIX_LOCO_MASK            1
#define MATRIX_WAGON_MASK           2
#define MATRIX_MARKERS_CONTOURS     3

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

    this->matrixPool = new MatrixPool();
    this->trackMatrixPool = new MatrixPool();
    this->handMatrixPool = new MatrixPool();

    this->videoProbe = new QVideoProbe();
    connect(this->videoProbe, SIGNAL(videoFrameProbed(QVideoFrame)), this, SLOT(processFrame(QVideoFrame)));
    connect(this->matrixPool, SIGNAL(debugMatrixReady(QImage)), this, SIGNAL(debugMatrixReady(QImage)));
    connect(this->trackMatrixPool, SIGNAL(debugMatrixReady(QImage)), this, SIGNAL(debugMatrixReady(QImage)));
    connect(this->handMatrixPool, SIGNAL(debugMatrixReady(QImage)), this, SIGNAL(debugMatrixReady(QImage)));
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

    this->matrixPool->addName(MATRIX_GRAY_ALL, "gray_all");
    this->matrixPool->addName(MATRIX_HSV_ALL, "hsv_all");
    this->matrixPool->addName(MATRIX_ADAPTIVE, "adaptive");
    this->trackMatrixPool->addName(MATRIX_TRACK_MASKED, "track_masked");
    this->trackMatrixPool->addName(MATRIX_LOCO_MASK, "loco_mask");
    this->trackMatrixPool->addName(MATRIX_WAGON_MASK,"wagon_mask");
    this->trackMatrixPool->addName(MATRIX_MARKERS_CONTOURS,"markers_contours");
    this->handMatrixPool->addName(MATRIX_DIFFERENCE, "difference");

    this->staticImage = cv::Mat(h,w,CV_8UC4);

    this->handWorker = new FrameProcessingWorker([this](auto wi){ this->workOnHandDetection(wi.original, wi.hsv, wi.gray, wi.adaptive); });
    this->trackWorker = new FrameProcessingWorker([this](auto wi){ this->workOnTrackDetection(wi.original, wi.hsv, wi.gray, wi.adaptive); });
}


VisionService::~VisionService()
{
    this->handWorker->join();
    this->trackWorker->join();
    delete this->matrixPool;
    delete this->videoProbe;
    delete this->handWorker;
    delete this->trackWorker;
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

std::vector<cv::RotatedRect> VisionService::getObjects(int matrixPoolIndex, std::shared_ptr<cv::Mat> img, DetectionSpecs specs, Contours& contours)
{
    std::vector<cv::RotatedRect> ret;

    std::shared_ptr<cv::Mat> mat = this->trackMatrixPool->getMatrix(matrixPoolIndex);
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

        // TODO: we're only searching in first level when doing that. Should iteratate sequentially instead
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

bool VisionService::detectWagons(std::shared_ptr<cv::Mat> mat)
{
    Contours contours;

    std::vector<cv::RotatedRect> wagons = this->getObjects(MATRIX_WAGON_MASK, mat, this->wagonSpecs, contours);
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

bool VisionService::detectLocomotive(std::shared_ptr<cv::Mat> mat, DetectionSpecs& specs)
{
    Contours contours;

    std::vector<cv::RotatedRect> points = this->getObjects(MATRIX_LOCO_MASK, mat, specs, contours);

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


void VisionService::detectMarkers(std::shared_ptr<cv::Mat> mat, std::shared_ptr<cv::Mat> adaptive)
{
    Contours contours;

    cv::Mat tmp;
    std::shared_ptr<cv::Mat> cntmat = this->trackMatrixPool->getMatrix(MATRIX_MARKERS_CONTOURS);
    cv::Mat mc(adaptive.get()->size(),CV_8UC1);
    mc.setTo(0);
    cntmat.get()->create(adaptive.get()->size(), CV_8UC1);
    erode(*adaptive, tmp, cv::Mat());
    erode(tmp, *adaptive, cv::Mat());
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(*adaptive, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_NONE);
    for (int contourIndex = 0; contourIndex < contours.size(); contourIndex++)
    {
        auto contour = contours[contourIndex];

        cv::RotatedRect rr = cv::minAreaRect(contour);
        double area = cv::contourArea(contour);
        if (rr.size.area() <2000 || rr.size.area() >3500) continue;
        double ratio = rr.size.width/rr.size.height;
        if (ratio > 1.2 || ratio < 0.8) continue;

        cv::Rect r = cv::boundingRect(contour);
        cv::Mat source = (*adaptive)(r);
        source = source.clone();
        cv::Mat rotated(r.height,r.width,CV_8UC1);
        rotated.setTo(0);
        cv::Point2f center(r.width/2,r.height/2);
        float angle = rr.angle+90;
        if (rr.size.width > rr.size.height) angle-=90;
        cv::Mat rotation = cv::getRotationMatrix2D(center, angle, 1);
        warpAffine(source, rotated, rotation, rotated.size(), cv::INTER_CUBIC,cv::BORDER_CONSTANT, cv::Scalar(255));

        cv::bitwise_not(rotated,rotated);

        double dw = (r.width- rr.size.width);
        double dh = (r.height- rr.size.height);
        if (dw <0) dw = 0;
        if (dh < 0) dh = 0;
        tmp = rotated(cv::Rect(dw,dh,rotated.size().width-dw*2,rotated.size().height-dh*2));
        rotated = tmp;
        cv::Mat target = this->generateCrossingTemplate(rotated.size().width, rotated.size().height);

        // We mask the candidate with a template that we are looking for. Then we calculate the difference
        // between the target and the masked target. The difference should be small
        double n = cv::norm(rotated,target, cv::NORM_L2, target);
        if (n > 3000) continue;
        tmp.copyTo(mc.colRange(r.x,r.x+rotated.size().width).rowRange(r.y,r.y+rotated.size().height));

        DetectedMarker dm;
        dm.pos = QPoint(rr.center.x, rr.center.y);
        dm.code = MARKER_TYPE_CROSSING;
        emit markerFound(dm);
    }
    mc.copyTo(*cntmat);
}

cv::Mat VisionService::generateCrossingTemplate(int w, int h)
{
    cv::Mat target(h,w, CV_8UC1);
    target.setTo(0);
    cv::Point pt1, pt2;
    pt1.x = 0;
    pt1.y = 0;
    pt2.x = target.size().width;
    pt2.y = target.size().height;
    cv::line(target, pt1, pt2, cv::Scalar(255), 3, cv::LINE_AA);
    pt1.x = target.size().width;
    pt1.y = 0;
    pt2.x = 0;
    pt2.y = target.size().height;
    cv::line(target, pt1, pt2, cv::Scalar(255), 3, cv::LINE_AA);
    cv::rectangle(target,cv::Rect(0,0,w, h), cv::Scalar(255), 3);

    return target;
}


bool VisionService::detectHand(std::shared_ptr<cv::Mat> mat)
{

/*    
//TODO: Do this in another thread, on another core

    Contours contours;
    QVector<QPoint> fingers;
    std::vector<cv::Vec4i> defects;
    std::vector<int> hull;
    std::vector<int> reducedHull;
    std::vector<std::pair<int, int>> candidates;
    Contour contour;
    cv::Point center;

    cv::Mat* handMat = this->matrixPool.getMatrix("hand_detect");
    cv::Mat* handContoursMat = this->matrixPool.getMatrix("hand_contours");
    handContoursMat->create(mat->size(), CV_8UC4);
    cv::inRange(*mat, HSV(10,30,40), HSV(40,100,80), *handMat);
    //cv::inRange(*hsvImage, HSV(20,30,0), HSV(50,100,80), *handMat);  // Skin color
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(*handMat, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

    for (int contourIndex = 0; contourIndex < contours.size(); contourIndex++)
    {
        contour = contours[contourIndex];
        cv::RotatedRect rr = cv::minAreaRect(contour);
        double area = cv::contourArea(contour);
        if (rr.size.area() <5000) continue; // TODO: MAGIC NUMBER


        center.x = rr.center.x;
        center.y = rr.center.y;
        cv::convexHull(contour, hull, true, true);
        if (hull.size() < 3) continue;
        break;
    }

    if (hull.size() <3) return false;

    // Find all points that are close to each other in the hull
    std::vector<int> labels;
    int labelCount = cv::partition(hull, labels, [contour](int ia, int ib) {
        cv::Point a = contour[ia];
        cv::Point b = contour[ib];
        return (((a.x-b.x)*(a.x-b.x))+((a.y-b.y)*(a.y-b.y))) < 30; //TODO: MAGIC NUMBER
    });

    // Cluster those points together by keeping the first one we find.
    int currentLabel = 0;
    while (labelCount)
    {
        for (int i = 0; i < labels.size(); i++)
        {
            if (labels[i] == currentLabel)
            {
                reducedHull.push_back(hull.at(i));
                break;
            }
        }
        labelCount--;
        currentLabel++;
    }
    cv::convexityDefects(contour, reducedHull, defects);

    if (!defects.size()) return false;

    if (this->matrixPool.getDebug())
    {
        Contours cc;
        cc.push_back(contour);
        cv::drawContours(*handContoursMat, cc, 0, cv::Scalar(255,255,255));
    }

    cv::Point lastDefect;
    lastDefect.x = -1;
    for (auto d : defects)
    {
        cv::Point pstart = contour[d[0]];
        cv::Point pend = contour[d[1]];
        cv::Point pfar = contour[d[2]];
        int depth = d[3];
        if (depth < 6000) continue;

        // the depth between the concave point and the hull should be large enough
        if (this->matrixPool.getDebug()) cv::circle(*handContoursMat, pfar, 5, cv::Scalar(255,0,0));

        auto color = cv::Scalar(0,255,255);

        if (lastDefect.x == -1) candidates.push_back({d[0], d[2]});
        candidates.push_back({d[1], d[2]});
        lastDefect = pfar;
    }

    for (int i = 0; i < candidates.size(); i++)
    {
        cv::Point cvp = contour[candidates[i].first];
        QPoint p = QPoint(cvp.x, cvp.y);

        int i1 = candidates[i].first+20;
        int i2 = candidates[i].first-20;
        if (i1> contour.size()) i1 -=contour.size();
        if (i2 < 0) i2 += contour.size();

        cv::Point p1 = contour[i1];
        cv::Point p2 = contour[i2];

        double fingerWidth = CVNORM(p1.x, p1.y, p2.x, p2.y);
        if (fingerWidth < 50)
        {
            if (this->matrixPool.getDebug()) cv::circle(*handContoursMat, cvp, 5, cv::Scalar(0,0,255));
            fingers.append(p);
        }

        cv::Point cvpf = contour[candidates[i].second];
        if (this->matrixPool.getDebug())
        {
            cv::line(*handContoursMat, center, cvpf, cv::Scalar(0,255,0), 1);
            cv::line(*handContoursMat, cvpf, cvp, cv::Scalar(0,255,255), 1);
            cv::line(*handContoursMat, p1, p2, cv::Scalar(0,0, 255), 1);
        }
    }

    if (fingers.size()) emit fingersDetected(fingers);

*/
    return true;
}


void VisionService::processFrame(QVideoFrame frame)
{
    cv::Point2f v2f[4];
    cv::Point v[4];
    cv::Mat tmp, tmp2, tmp3;

    this->matrixPool->startWorking();

    // The frame comming from the camera are RGB32. Other cams could return something else so we'd need to adjust this.
    frame.map(QAbstractVideoBuffer::ReadOnly);

    std::shared_ptr<cv::Mat> original = this->matrixPool->getMatrix(MATRIX_ORIGINAL);
    std::shared_ptr<cv::Mat> grayImage = this->matrixPool->getMatrix(MATRIX_GRAY_ALL);
    std::shared_ptr<cv::Mat> hsvImage = this->matrixPool->getMatrix(MATRIX_HSV_ALL);

    *original = cv::Mat(frame.height(), frame.width(), CV_8UC4, frame.bits());

    grayImage.get()->create(frame.height(), frame.width(), CV_8UC1);
    hsvImage.get()->create(frame.height(), frame.width(), CV_8UC4);
    cv::cvtColor(*original, *grayImage.get(), cv::COLOR_BGR2GRAY);
    cv::cvtColor(*original, *hsvImage.get(), cv::COLOR_BGR2HSV);
    std::shared_ptr<cv::Mat> adaptive = this->matrixPool->getMatrix(MATRIX_ADAPTIVE);
    cv::adaptiveThreshold(*grayImage.get(), *adaptive.get(), 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 5,3);

    WorkItem wi;
    wi.hsv = hsvImage;
    wi.gray = grayImage;
    wi.adaptive = adaptive;
    wi.original = original;
    this->handWorker->queueWork(wi);
    this->trackWorker->queueWork(wi);

    this->matrixPool->stopWorking();
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


void VisionService::disableDebug()
{
    this->matrixPool->enableDebug(-1);
}

void VisionService::enableDebug(QString name)
{
    this->matrixPool->enableDebug(-1);
    this->trackMatrixPool->enableDebug(-1);
    this->handMatrixPool->enableDebug(-1);

    auto names = this->matrixPool->getNames();
    for (int key : names.keys())
    {
        if (names[key] == name)
        {
            this->matrixPool->enableDebug(key);
            return;
        }
    }
    names = this->trackMatrixPool->getNames();
    for (int key : names.keys())
    {
        if (names[key] == name)
        {
            this->trackMatrixPool->enableDebug(key);
            return;
        }
    }
    names = this->handMatrixPool->getNames();
    for (int key : names.keys())
    {
        if (names[key] == name)
        {
            this->handMatrixPool->enableDebug(key);
            return;
        }
    }

}

QVector<QString> VisionService::getDebugNames()
{
    QVector<QString> ret;
    for (QString name : this->matrixPool->getNames())
    {
        ret.append(name);
    }
    for (QString name : this->trackMatrixPool->getNames())
    {
        ret.append(name);
    }
    for (QString name : this->handMatrixPool->getNames())
    {
        ret.append(name);
    }

    return ret; 
}

void VisionService::workOnTrackDetection(std::shared_ptr<cv::Mat> original, std::shared_ptr<cv::Mat> hsv, std::shared_ptr<cv::Mat> gray, std::shared_ptr<cv::Mat> adaptive)
{
    PerformanceMonitor::tic("VisionService::workOnTrackDetection");
    this->trackMatrixPool->startWorking();

    // Get the track-masked image
    cv::Mat tmp;
    std::shared_ptr<cv::Mat> masked = this->trackMatrixPool->getMatrix(MATRIX_TRACK_MASKED);
    hsv.get()->copyTo(*masked, this->trackMask);

    bool locoDetected = false;

    if (this->annotationDetectionEnabled)
    {
        this->detectMarkers(masked, adaptive);
    }

    // Detect wagons on the masked image
    this->detectWagons(masked);

    // Get locomotive
    if (this->restrictLocomotiveDetectionToTracks)
    {
        locoDetected = this->detectLocomotive(masked, this->locomotiveSpecs);
    }
    else
    {
        locoDetected = this->detectLocomotive(hsv, this->locomotiveSpecsOffTracks);
    }

    emit frameProcessed(this->locomotive(), this->wagons());

    this->trackMatrixPool->stopWorking();
    PerformanceMonitor::toc("VisionService::workOnTrackDetection");
}

void VisionService::workOnHandDetection(std::shared_ptr<cv::Mat> original, std::shared_ptr<cv::Mat> hsv, std::shared_ptr<cv::Mat> gray, std::shared_ptr<cv::Mat> adaptive)
{
    cv::Mat tmp;
    cv::Mat tmp2;

    PerformanceMonitor::tic("VisionService::workOnHandDetection");
    this->handMatrixPool->startWorking();


    std::shared_ptr<cv::Mat> diff = this->handMatrixPool->getMatrix(MATRIX_DIFFERENCE);
    //diff->create(VIDEO_HEIGHT, VIDEO_WIDTH, CV_8UC1);
    cv::absdiff(this->staticImage, *original, tmp2);
    cv::cvtColor(tmp2, tmp, cv::COLOR_BGR2GRAY);
    if (cv::mean(tmp)[0] > 60)
    {
        // If the background changed drastically, update it
        original->copyTo(this->staticImage);
    }

//    cv::adaptiveThreshold(tmp, *diff, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 5,3);
    cv::threshold(tmp, *diff, 40, 255, 0);




    this->handMatrixPool->stopWorking();
    //this->detectHand(hsv);
    PerformanceMonitor::toc("VisionService::workOnHandDetection");
}
