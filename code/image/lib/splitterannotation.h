#ifndef SPLITTERANNOTATION_H
#define SPLITTERANNOTATION_H

#include "annotation.h"

class SplitterAnnotation : public Annotation
{
public:
    SplitterAnnotation();

    QString pixmapName() override;

    void setTrack1(QString val);
    QString getTrack1();
    void setTrack2(QString val);
    QString getTrack2();
    void setInputTrack(QString val);
    QString getInputTrack();
    void setClockWise(bool val);
    bool getClockWise();

    void serialize(QDataStream& ds) override;
    void deSerialize(QDataStream& ds) override;

private:

    QString track1;
    QString track2;
    QString inputTrack;
    bool clockWise;

};

#endif // SPLITTERANNOTATION_H
