#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QVector>
#include "track.h"
#include "annotation.h"
#include "crossroadannotation.h"
#include "splitterannotation.h"

class Configuration
{
private:
    QVector<Track*> tracks;
    QVector<Annotation*> annotations;
    QString controllerPort;
    void assignRelayToAnnotations();
    int lightLevel;
    int soundLevel;

public:
    Configuration();
    Configuration(QString trackFile, QString configFile);

    void addTrack(Track* track);
    void removeTrack(Track* track);
    QVector<Track*> getTracks();
    Track* getTrack(const QString& name);
    void load(QString trackFile, QString configFile);

    void addAnnotation(Annotation* a);
    void removeAnnotation(Annotation* a);
    QVector<Annotation*> getAnnotations();

    QString getControllerPort();
    void save();

    void setSoundLevel(int val);
    int getSoundLevel();

    void setLightLevel(int val);
    int geLightLevel();

    void clearSplitterAnnotations();

    QVector<CrossRoadAnnotation*> getCrossroadsAnnotations();
    QVector<SplitterAnnotation*> getSplitterAnnotations();

};

#endif // CONFIGURATION_H
