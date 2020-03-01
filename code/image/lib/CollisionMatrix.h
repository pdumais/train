#ifndef COLLISIONMATRIX_H
#define COLLISIONMATRIX_H

#include <QImage>
#include <QMap>
#include <QPainter>
#include <QPoint>
#include "constants.h"

/*
 *  This is not used at the moment because the the performance gain
 *  is really not required.
 *  The goal was to:
 *   handle the video feed as a 32x32 grid. Convert train points to points on the grid
 *      but also mark neighbours if the real point is close to them
 *      neightbourUp is set if realpoint.y - (w/2) falls into neighbourUp
 *            set each grid point within ((w/2)-1) of x,y to pointer to object
 *           grid[(x/w)+(y/w)*32]
 *           grid[(x-1/w)+(y-1/w)*32]
 *           grid[(x-1/w)+(y+1/w)*32]
 *           grid[(x+1/w)+(y-1/w)*32]
 *           grid[(x+!/w)+(y+1/w)*32]
 *           d = hypotenuse of w
 *               then do all 4 squares with +/- d
 *  then we can do collision detection by comparing grids.
 *
 *
 */

template<class T>
using RawCollisionMatrix = QMap<int,T>;

template<class T, int w, int h>
class CollisionMatrix
{
public:
    explicit CollisionMatrix(RawCollisionMatrix<bool> mask)
    {
        for (auto it : mask.keys())
        {
            this->data[it] = new QVector<T>();
        }
    }

    ~CollisionMatrix()
    {
        for (auto it : this->data.values())
        {
            delete it;
        }
        this->data.clear();
    }

    static QPoint getCorrespondingSlot(QPoint p)
    {
        return QPoint(p.x()/(VIDEO_WIDTH/w), p.y()/(VIDEO_HEIGHT/h));
    }

    void addPoint(QPoint p, T val)
    {
        QPoint lowResPoint = CollisionMatrix<T,w,h>::getCorrespondingSlot(p);
        if (!this->data.contains(lowResPoint.x()+(lowResPoint.y()*32))) return;
        QVector<T>* v = this->data[lowResPoint.x()+(lowResPoint.y()*32)];
        if (!v)
        {
            // Don't add this point because it was masked;
            return;
        }

        v->append(val);
    }

    void drawToDebugImage(QImage img, QColor col)
    {
        QPainter p(&img);

        int vw = (VIDEO_WIDTH/w);
        int vh = (VIDEO_HEIGHT/h);
        p.setBrush(col);
        for (auto key : this->data.keys())
        {
            int x = (key % w)*vw;
            int y = ((key)/w)*vh;

            p.fillRect(x-(vw/2),y-(vh/2),vw,vh,col);
        }

        //TODO: draw this to an image and we would overlay that image in the displayService
    }

    template<class T2>
    QVector<T> getCollisions(CollisionMatrix<T2,w,h> target)
    {
        QVector<T> ret;

        for (auto key : target.data.keys())
        {
            if (this->data[key])
            {
                // TODO: dont add duplicates. So use Map instead?
                ret.append(this->data[key]);
            }
        }

        return ret;
    }

private:
    RawCollisionMatrix<QVector<T>*> data;
};

template<class T>
using CollisionMatrix32x32 = CollisionMatrix<T,32,32>;


#endif // COLLISIONMATRIX_H
