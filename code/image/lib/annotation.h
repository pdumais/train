#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <QObject>
#include <QPoint>
#include <QDataStream>


class Annotation : public QObject
{
    Q_OBJECT
public:
    explicit Annotation(QObject *parent = nullptr);

    void setName(QString  val);
    QString getName();

    void setPosition(QPoint val);
    QPoint getPosition();

    virtual QString pixmapName() = 0;
    virtual void serialize(QDataStream& ds) = 0;
    virtual void deSerialize(QDataStream& ds) = 0;

    int getRelayCount();
    void setRelay(int addr, int val);
    int getRelay(int addr);

    bool setInRange(bool val);
    bool getInRange();
    int getRadius();

protected:
    QString name;
    QPoint position;
    int relayCount;
    int relay[10];
    bool inRange;
    int radius;
};

Q_DECLARE_METATYPE(Annotation*);

QDataStream &operator<<(QDataStream &out, Annotation* t);
QDataStream &operator>>(QDataStream &in, Annotation* &t);

#endif // ANNOTATION_H
