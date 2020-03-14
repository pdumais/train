#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QPolygon>
#include <functional>

class Functions
{
public:
    Functions();

    static uint32_t perimeter(QPolygon poly);
    static void nonBlockingSleep(int ms);

    static void overrideSleepFunction(std::function<void(int)>& fn);
private:
    static std::function<void(int)> sleepFunction;    
};

void init_app();

#endif // FUNCTIONS_H
