#pragma once
#include <QObject>
#include <thread>
#include <functional>
#include <QQueue>
#include <QMutex>
#include <QThread>
#include <QAtomicInt>
#include "opencv/cv.h"

struct WorkItem
{
    std::shared_ptr<cv::Mat> original;
    std::shared_ptr<cv::Mat> hsv;
    std::shared_ptr<cv::Mat> gray;
    std::shared_ptr<cv::Mat> adaptive;
};

class FrameProcessingWorker: public QObject
{
    Q_OBJECT
public:
    FrameProcessingWorker(std::function<void(WorkItem)> f);
    ~FrameProcessingWorker();

    void queueWork(WorkItem);
    void join();

public slots:
    void work(WorkItem);

private:
    QAtomicInt backlogCounter;
    QThread thread;
    std::function<void(WorkItem)> callback;

};

Q_DECLARE_METATYPE(WorkItem);
