#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QPolygon>
#include <functional>
#include <any>

class anycast
{
private:
    std::any a;
public:
    anycast(std::any a) { this->a = a; };
    template<typename T>
    operator T() {
        if (!a.has_value()) return T();
        return std::any_cast<T>(a);
    }
    template<typename T>
    operator T*() {
        if (!a.has_value()) return nullptr;
        return std::any_cast<T*>(a);
    }
};



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
