#include "FrameProcessingWorker.h"
#include <QMetaObject>
#include <QDebug>

FrameProcessingWorker::FrameProcessingWorker(std::function<void(WorkItem)> f): QObject(nullptr)
{
    this->callback = f;

    this->backlogCounter = 0;
    this->moveToThread(&this->thread);
    this->thread.start();
}

FrameProcessingWorker::~FrameProcessingWorker()
{
}

void FrameProcessingWorker::queueWork(WorkItem wi)
{
    //we make sure we dont queue faster than we dequeue.
    // Otherwise the eventloop will accumulate to infinity
    if (this->backlogCounter.loadAcquire() >= 1)
    {
        qDebug() << "WARNING: Discarding frame";
        return;
    }
    
    this->backlogCounter++;

    // This will invoke work() on the thread
    QMetaObject::invokeMethod(this, "work", Qt::QueuedConnection, Q_ARG(WorkItem, wi));
}

void FrameProcessingWorker::join()
{
    this->thread.exit();
    this->thread.wait();
}

void FrameProcessingWorker::work(WorkItem wi)
{
    //It's ok if another event gets pooled while we're in this one.
    this->backlogCounter--;

    this->callback(wi);
}
