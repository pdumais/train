#include "configuration.h"
#include "splitterannotation.h"

#include <QFile>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include "constants.h"

#define CONFIG_FILE "/home/pat/projects/train/data/config.json"
#define TRACK_DATA "/home/pat/projects/train/data/tracks.dat"

Configuration::Configuration()
{
}

Configuration::Configuration(QString trackFile, QString configFile)
{
    this->load(trackFile, configFile);
}

void Configuration::addTrack(Track* track)
{
    this->tracks.append(track);
}

void Configuration::removeTrack(Track* track)
{
    int index = this->tracks.indexOf(track);
    if (index == -1) return;

    this->tracks.remove(index);
    delete track;
}

QVector<Track*> Configuration::getTracks()
{
    return this->tracks;
}

void Configuration::addAnnotation(Annotation* a)
{
    this->annotations.append(a);
    this->assignRelayToAnnotations();
}

void Configuration::removeAnnotation(Annotation* a)
{
    int index = this->annotations.indexOf(a);
    if (index == -1) return;

    this->annotations.remove(index);
    this->assignRelayToAnnotations();
    delete a;
}

void Configuration::assignRelayToAnnotations()
{
    qSort(this->annotations.begin(), this->annotations.end(), [](Annotation* a, Annotation* b) {
        return ((a->getPosition().y()*VIDEO_WIDTH)+a->getPosition().x()) > ((b->getPosition().y()*VIDEO_WIDTH)+b->getPosition().x());
    });

    int rc = FIRST_CROSSROAD_RELAY-'a'+1;
    int rs = 1;
    for (auto a : this->annotations)
    {
        if (dynamic_cast<CrossRoadAnnotation*>(a))
        {
            for (int i = 0; i < a->getRelayCount(); i++)
            {
                a->setRelay(i, rc);
                rc++;
            }
        }
        else if (dynamic_cast<SplitterAnnotation*>(a))
        {
            for (int i = 0; i < a->getRelayCount(); i++)
            {
                if (rs < FIRST_CROSSROAD_RELAY)
                {
                    a->setRelay(i, rs);
                    rs++;
                }
            }
        }
    }
}

QVector<Annotation*> Configuration::getAnnotations()
{
    return this->annotations;
}

void Configuration::load(QString trackFile, QString configFile)
{
    QFile f(configFile);
    if (!f.open(QIODevice::ReadOnly)) return;

    QByteArray data = f.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));
    this->controllerPort = doc["controller"]["port"].toString();
    qDebug() << "Controller Port: " << controllerPort;

    QFile f2(trackFile);
    f2.open(QIODevice::ReadOnly);
    QDataStream in(&f2);
    in >> this->tracks >> this->soundLevel >> this->lightLevel;

    this->assignRelayToAnnotations();
}

QString Configuration::getControllerPort()
{
    return this->controllerPort;
}

void Configuration::save()
{
    QFile f(TRACK_DATA);
    f.open(QIODevice::ReadWrite);

    QDataStream out(&f);
    out << this->tracks << this->soundLevel << this->lightLevel;

}

Track* Configuration::getTrack(const QString& name)
{
    for (Track* t : this->tracks)
    {
        if (t->getName() == name) return t;
    }

    return nullptr;
}

QVector<SplitterAnnotation*> Configuration::getSplitterAnnotations()
{
    QVector<SplitterAnnotation*>  ret;
    for (Annotation* a : this->annotations)
    {
        SplitterAnnotation* sa = dynamic_cast<SplitterAnnotation*>(a);
        if (sa) ret.append(sa);
     }

     return ret;
}

QVector<CrossRoadAnnotation*> Configuration::getCrossroadsAnnotations()
{
    QVector<CrossRoadAnnotation*>  ret;
    for (Annotation* a : this->annotations)
    {
        CrossRoadAnnotation* ca = dynamic_cast<CrossRoadAnnotation*>(a);
        if (ca) ret.append(ca);
     }

     return ret;
}

void Configuration::setSoundLevel(int val)
{
    this->soundLevel = val;
}

int Configuration::getSoundLevel()
{
    return this->soundLevel;
}

void Configuration::setLightLevel(int val)
{
    this->lightLevel = val;
}

int Configuration::geLightLevel()
{
    return this->lightLevel;
}

void Configuration::clearSplitterAnnotations()
{
    this->annotations.erase(std::remove_if(this->annotations.begin(), this->annotations.end(), [](Annotation* a) {
        SplitterAnnotation* sa = dynamic_cast<SplitterAnnotation*>(a);
        if (sa)
        {
            delete sa;
            return true;
        }
        return false;
    }),this->annotations.end());
}
