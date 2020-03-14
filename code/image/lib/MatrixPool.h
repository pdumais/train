#pragma once

#include "opencv2/core/core.hpp"
#include <QObject>
#include <QString>
#include <QImage>
#include <QMap>
#include <memory>

#define MAX_MATRICES 15

class MatrixPool: public QObject
{
    Q_OBJECT

public:
    MatrixPool();
    ~MatrixPool();

    std::shared_ptr<cv::Mat> getMatrix(int id);
    void enableDebug(int id);
    bool getDebugEnabled();
    void addName(int id, QString name);
    QMap<int, QString> getNames();
    void startWorking();
    void stopWorking();

signals:
    void debugMatrixReady(QImage);

private:

    std::shared_ptr<cv::Mat>  debugMatrix;
    QString names[MAX_MATRICES];
    int debugIndex;
    int lockedDebugIndex;
};













