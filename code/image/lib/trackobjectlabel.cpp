#include "trackobjectlabel.h"

#include <QDrag>
#include <QDebug>
#include <QMimeData>
#include <QMouseEvent>

TrackObjectLabel::TrackObjectLabel(QWidget *parent): QLabel(parent)
{

}

void TrackObjectLabel::mousePressEvent(QMouseEvent *event)
{
    QMimeData *mimeData = new QMimeData();
    mimeData->setText(this->objectName());
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(this->pixmap()->scaled(QSize(32,32)));
    drag->setHotSpot(event->pos() - this->pos());

    Qt::DropAction dropAction = drag->exec();
}
