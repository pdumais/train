#ifndef TRACKOBJECTLABEL_H
#define TRACKOBJECTLABEL_H

#include <QLabel>

class TrackObjectLabel : public QLabel
{
public:
    TrackObjectLabel(QWidget *parent);

protected:
    void mousePressEvent(QMouseEvent *event) override;

};

#endif // TRACKOBJECTLABEL_H
