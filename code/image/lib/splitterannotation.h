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

    void setActiveTrack(QString val);
    QString getActiveTrack();

    void serialize(QDataStream& ds) override;
    void deSerialize(QDataStream& ds) override;

    QString toString();
private:

    QString track1;
    QString track2;
    QString inputTrack;
    QString activeTrack;
    bool clockWise;

};

#endif // SPLITTERANNOTATION_H
