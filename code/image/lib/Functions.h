#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QPolygon>

class Functions
{
public:
    Functions();

    static uint32_t perimeter(QPolygon poly);
};

#endif // FUNCTIONS_H
