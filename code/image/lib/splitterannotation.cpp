#include "splitterannotation.h"

#include <QTextStream>
#include <QVariant>
#include "constants.h"

SplitterAnnotation::SplitterAnnotation(): Annotation(nullptr)
{
    this->radius = SPLITTER_RADIUS;
    this->relay[0]=0;
    this->relay[1]=0;
    this->relayCount = 2;
    this->track1 = "";
    this->track2 = "";
    this->inputTrack = "";
    this->activeTrack = "";
    this->clockWise = false;

    this->setProperty("type",QVariant("SplitterAnnotation"));
}

void SplitterAnnotation::setActiveTrack(QString val)
{
    this->activeTrack = val;
}

QString SplitterAnnotation::getActiveTrack()
{
    return this->activeTrack;
}

QString SplitterAnnotation::pixmapName()
{
    return ":/switchtracks.png";
}

void SplitterAnnotation::setInputTrack(QString val)
{
    this->inputTrack = val;
}

QString SplitterAnnotation::getInputTrack()
{
    return this->inputTrack;
}

void SplitterAnnotation::setTrack1(QString val)
{
    this->track1 = val;
}

QString SplitterAnnotation::getTrack1()
{
    return this->track1;
}

void SplitterAnnotation::setTrack2(QString val)
{
    this->track2 = val;
}

QString SplitterAnnotation::getTrack2()
{
    return this->track2;
}


void SplitterAnnotation::serialize(QDataStream& ds)
{
    ds << this->name << this->position;
}

void SplitterAnnotation::deSerialize(QDataStream& ds)
{
    ds >> this->name >> this->position;
}

void SplitterAnnotation::setClockWise(bool val)
{
    this->clockWise = val;
}

bool SplitterAnnotation::getClockWise()
{
    return this->clockWise;
}

QString SplitterAnnotation::toString()
{
    QString str;
    QTextStream(&str) << this->inputTrack << "/" << this->track1 << "/" << this->track2;
    return str;
}

