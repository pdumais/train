#pragma once

#include "opencv2/core/core.hpp"
#include <QString>
#include <QImage>

#define MAX_MATRICES 15

class MatrixPool
{
public:
    MatrixPool();
    ~MatrixPool();

    void setContext(char* name);
    cv::Mat* getMatrix(char* name);
    void reset();
    void setDebug(bool val);
    QImage dumpMatrix(QString name);
    QVector<QString> getNames();
private:
    QString names[MAX_MATRICES];
    cv::Mat matrices[MAX_MATRICES];
    QString currentContext;
    int index;
    bool debug;
};













