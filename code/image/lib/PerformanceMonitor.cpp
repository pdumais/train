#include "PerformanceMonitor.h"
#include <QTime>
#include <QDebug>

#define LOG
#define COUNT_MAX 30

QMap<QString,PerformanceMonitor::PerformanceCounter> PerformanceMonitor::counters;

PerformanceMonitor::PerformanceMonitor()
{

}

void PerformanceMonitor::tic(QString name)
{
    if (!PerformanceMonitor::counters.contains(name))
    {
        PerformanceMonitor::counters[name] = PerformanceCounter{ QTime(), 0 };
    }

    PerformanceMonitor::counters[name].timer.restart();
}

void PerformanceMonitor::toc(QString name)
{


    PerformanceMonitor::counters[name].accumulator +=  PerformanceMonitor::counters[name].timer.elapsed();
    PerformanceMonitor::counters[name].count++;

    if (PerformanceMonitor::counters[name].count >= COUNT_MAX)
    {
#ifdef LOG
        qDebug() << "Performance Counter '" << name << "': " <<  (double(PerformanceMonitor::counters[name].accumulator)/double(PerformanceMonitor::counters[name].count)) << "ms for " << COUNT_MAX << " counts";
#endif
        PerformanceMonitor::counters[name].accumulator =0;
        PerformanceMonitor::counters[name].count = 0;
    }

}
