#include "annotation.h"
#include <QVariant>

#include "crossroadannotation.h"
#include "splitterannotation.h"
#include <QDebug>

Annotation::Annotation(QObject *parent) : QObject(parent)
{
    this->radius = 0;
    this->relayCount = 0;
    this->inRange = false;
}

int Annotation::getRadius()
{
    return this->radius;
}

void Annotation::setName(QString  val)
{
    this->name = val;
}

QString Annotation::getName()
{
    return this->name;
}

void Annotation::setPosition(QPoint val)
{
    this->position = val;
}

QPoint Annotation::getPosition()
{
    return this->position;
}

bool Annotation::setInRange(bool val)
{
    if (val == this->inRange) return false;
    this->inRange = val;
    return true;
}

bool Annotation::getInRange()
{
    return this->inRange;
}

int Annotation::getRelayCount()
{
    return this->relayCount;
}

void Annotation::setRelay(int addr, int val)
{
    this->relay[addr] = val;

    qDebug() << "Annotation [" << this->name << "] assigned relay " << this->relay[addr];
}

int Annotation::getRelay(int addr)
{
    return this->relay[addr];
}

QDataStream &operator<<(QDataStream &out, Annotation* t)
{
    QString type = t->property("type").value<QString>();
    out << type;
    t->serialize(out);

    return out;
}

QDataStream &operator>>(QDataStream &in, Annotation* &t)
{
    QString type;
    in >> type;

    if (type == "CrossRoadAnnotation")
    {
        t = new CrossRoadAnnotation();
    }
    else if (type == "SplitterAnnotation")
    {
        t = new SplitterAnnotation();
    }
    else
    {
        return in;
    }

    t->deSerialize(in);

    return in;
}
