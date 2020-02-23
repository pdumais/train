#include "cvobject.h"

CVObject::CVObject()
{
}

CVObject::CVObject(const CVObject &obj)
{
    if (&obj == this) return;
    this->poly = obj.poly;
    this->line = obj.line;
    this->center = obj.center;
}

CVObject::~CVObject()
{
}

QPolygon CVObject::getPolygon()
{
    return this->poly;
}

void CVObject::setPolygon(QPolygon val)
{
    this->poly = val;
}
    
QLine CVObject::getLine() const
{
    return this->line;
}

void CVObject::setLine(QLine val)
{
    this->line = val;
}

QPoint CVObject::getCenter() const
{
    return this->center;
}

void CVObject::setCenter(QPoint val)
{
    this->center = val;
}
