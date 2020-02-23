#ifndef QDRAGDROPGRAPHICSSCENE_H
#define QDRAGDROPGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPoint>


class QDragDropGraphicsScene: public QGraphicsScene
{
    Q_OBJECT
public:
    QDragDropGraphicsScene();

signals:
    void objectDropped(QString,QPoint);
    void mouseClick(QPoint);

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
//    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;
};

#endif // QDRAGDROPGRAPHICSSCENE_H
