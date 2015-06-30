#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QWidget>
//#include <QPixmap>
#include <QPainter>

class RenderArea : public QWidget
{
    Q_OBJECT

public:
    RenderArea(QWidget *parent = 0);
    void setLevel(qreal value);

public slots:
    void changeLevel(qreal value);

protected:
    void paintEvent(QPaintEvent *event);

private:
    qreal level;
//    QPixmap pixmap;
    int addTop;
    int addBottom;
    int addLeft;
    int addRight;
};


#endif //RENDERAREA_H

