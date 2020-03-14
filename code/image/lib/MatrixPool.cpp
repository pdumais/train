#include "MatrixPool.h"

#include <QDebug>
#include <exception>

MatrixPool::MatrixPool(): QObject(nullptr)
{
    this->debugIndex = -1;
    this->debugMatrix = std::make_shared<cv::Mat>();
}

MatrixPool::~MatrixPool()
{
}

void MatrixPool::addName(int id, QString name)
{
    if (id >= MAX_MATRICES)
    {
        throw std::exception();
    }

    if (!this->names[id].isEmpty())
    {
        throw std::exception();
    }

    this->names[id] = name;
}

void MatrixPool::enableDebug(int id)
{
    // This value will only picked up when startWorking() is called
    // so it is safe to change it while the thread is working on it.
    this->debugIndex = id;
}

std::shared_ptr<cv::Mat> MatrixPool::getMatrix(int id)
{
    if (this->lockedDebugIndex == id)
    {
        return this->debugMatrix;
    }

    return std::make_shared<cv::Mat>();
}

bool MatrixPool::getDebugEnabled()
{
    return (this->debugIndex != -1);
}

QMap<int, QString> MatrixPool::getNames()
{
    QMap<int, QString> ret;
    for (int i = 0; i < MAX_MATRICES; i++)
    {
        QString str = this->names[i];
        if (str.isEmpty()) continue;
        ret[i] = str;       
    }
    return ret;
}

void MatrixPool::startWorking()
{
    // No need to lock here since changing a 64bit int is an atomic operation all the time any way
    this->lockedDebugIndex = this->debugIndex;
}

void MatrixPool::stopWorking()
{
    if (this->debugIndex == -1) return;
    cv::Mat* img = this->debugMatrix.get();
    emit debugMatrixReady(QImage(img->data,img->cols,img->rows, static_cast<int>(img->step), (img->type()==CV_8UC1)?QImage::Format_Grayscale8:QImage::Format_RGB32));
}

