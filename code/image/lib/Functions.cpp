#include "Functions.h"
#include "IVisionService.h"
#include "cvobject.h"
#include <QTimer>
#include <QEventLoop>  
#include <QMetaType>


std::function<void(int)> Functions::sleepFunction;    

Functions::Functions()
{

}

void Functions::nonBlockingSleep(int ms)
{
    if (Functions::sleepFunction)
    {
        Functions::sleepFunction(ms);
        return;
    }

    QEventLoop loop;
    QTimer::singleShot(300, &loop, &QEventLoop::quit);
    loop.exec();
}

void Functions::overrideSleepFunction(std::function<void(int)>& fn)
{
    Functions::sleepFunction = fn;
}

uint32_t Functions::perimeter(QPolygon poly)
{
    uint32_t ret = 0;
    if (poly.isEmpty()) return 10000000;
    if (poly.size() < 2) return 0;

    QPoint last = poly.first();
    for (QPoint p : poly)
    {
        ret += (p-last).manhattanLength();
        last = p;
    }

    return ret;
}

void init_app()
{
    qRegisterMetaType<CVObject>("CVObject");
    qRegisterMetaType<DetectedMarker>("DetectedMarker");
    qRegisterMetaType<QVector<CVObject>>("QVector<CVObject>");

}
