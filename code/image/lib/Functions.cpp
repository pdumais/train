#include "Functions.h"

Functions::Functions()
{

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
