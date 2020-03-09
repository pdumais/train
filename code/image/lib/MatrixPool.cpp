#include "MatrixPool.h"

#include <QDebug>

MatrixPool::MatrixPool()
{
    this->debug = false;
    this->reset();
}

MatrixPool::~MatrixPool()
{
}

void MatrixPool::setDebug(bool val)
{
    this->debug = val;
}

void MatrixPool::setContext(char* name)
{
    if (!this->debug) return;

    this->currentContext = name;
}

// The name is not used to retrieve a specific matrix, it is just used to tag the matrix that will be retrieved
cv::Mat* MatrixPool::getMatrix(char* name)
{
    int i = this->index;
    this->index++;
    if (this->index >= MAX_MATRICES)
    {
        qDebug() << "ERROR: Using too many matrices";
    }

    if (this->debug)
    {
        this->names[i] = this->currentContext + ":" + name;
    }
    this->matrices[i].setTo(0);
    return &this->matrices[i];
}

void MatrixPool::reset()
{
    this->index =0;
}

bool MatrixPool::getDebug()
{
    return this->debug;
}

QVector<QString> MatrixPool::getNames()
{
    QVector<QString> ret;
    for (int i = 0; i < MAX_MATRICES; i++)
    {
        ret.append(this->names[i]);
    }
    return ret;
}

QImage MatrixPool::dumpMatrix(QString name)
{
    for (int i = 0; i < MAX_MATRICES; i++)
    {
        if (this->names[i] == name)
        {
            cv::Mat img = this->matrices[i];
            return QImage(img.data,img.cols,img.rows, static_cast<int>(img.step), (img.type()==CV_8UC1)?QImage::Format_Grayscale8:QImage::Format_RGB32);
        }
    }

    return QImage();
}

