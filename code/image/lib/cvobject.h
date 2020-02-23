#pragma once

#include <QPolygon>
#include <QLine>
#include <QMetaType>

class CVObject
{
public:
    CVObject();
    CVObject(const CVObject &obj);
    ~CVObject();

    QPolygon getPolygon();
    void setPolygon(QPolygon val);

    QLine getLine() const;
    void setLine(QLine val);

    QPoint getCenter() const;
    void setCenter(QPoint val);

private:
    QPolygon poly;
    QPoint center;
    QLine  line;
};

Q_DECLARE_METATYPE(CVObject);

