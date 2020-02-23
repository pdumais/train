#include "crossroadannotation.h"

#include <QVariant>
#include "constants.h"

CrossRoadAnnotation::CrossRoadAnnotation(): Annotation(nullptr)
{
    this->radius = CROSSROAD_RADIUS;
    this->relay[0]=0;
    this->relayCount = 1;

    this->setProperty("type",QVariant("CrossRoadAnnotation"));
}

QString CrossRoadAnnotation::pixmapName()
{
    return ":/crossing.png";
}

void CrossRoadAnnotation::serialize(QDataStream& ds)
{
    ds << this->name << this->position;
}

void CrossRoadAnnotation::deSerialize(QDataStream& ds)
{
    ds >> this->name >> this->position;
}
