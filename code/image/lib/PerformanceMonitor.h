#ifndef PERFORMANCEMONITOR_H
#define PERFORMANCEMONITOR_H

#include <QMap>
#include <QMutex>
#include <QTime>


class PerformanceMonitor
{
public:
    PerformanceMonitor();

    static void tic(QString name);
    static void toc(QString name);

private:
    struct PerformanceCounter
    {
        QTime timer;
        uint32_t accumulator;
        uint32_t count;
    };

    static QMap<QString,PerformanceCounter> counters;
    static QMutex mapLock;
};

#endif // PERFORMANCEMONITOR_H
