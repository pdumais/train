#ifndef CROSSROADANNOTATION_H
#define CROSSROADANNOTATION_H

#include "annotation.h"

class CrossRoadAnnotation : public Annotation
{
public:
    CrossRoadAnnotation();

    QString pixmapName() override;

    void serialize(QDataStream& ds) override;
    void deSerialize(QDataStream& ds) override;

};

#endif // CROSSROADANNOTATION_H
