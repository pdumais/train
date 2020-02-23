#include "qdragdropgraphicsscene.h"

#include <QGraphicsItem>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsView>
#include <QMimeData>
#include <QPixmap>
#include <QDebug>

QDragDropGraphicsScene::QDragDropGraphicsScene(): QGraphicsScene()
{
}

void QDragDropGraphicsScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setAccepted(true);
}

void QDragDropGraphicsScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setAccepted(true);
}

void QDragDropGraphicsScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();

    QPoint pos = event->scenePos().toPoint();
    emit objectDropped(mimeData->text(), pos);

    event->setAccepted(true);
}

void QDragDropGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    // This is just to debug colors using breakpoints
    QPointF pos = mouseEvent->scenePos();
    QGraphicsView* v = this->views()[0];

    QPixmap pixMap = QPixmap::grabWidget(v);
    QColor col = pixMap.toImage().pixelColor(pos.toPoint());
    qDebug() << pos << " - " << col.toHsv();


    emit mouseClick(pos.toPoint());

}
