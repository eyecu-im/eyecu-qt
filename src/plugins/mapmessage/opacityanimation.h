#ifndef OPACITYANIMATION_H
#define OPACITYANIMATION_H

#include <QAbstractAnimation>
#include <QGraphicsItem>

class OpacityAnimation : public QAbstractAnimation
{
    Q_OBJECT
public:
    OpacityAnimation(int ADuration, QObject *parent = 0);

public:
    void setGraphicsItem(QGraphicsItem *AItem) {FItem = AItem;}
    // QAbstractAnimation interface
    virtual int duration() const {return FDuration;}

protected:
    virtual void updateCurrentTime(int ACurrentTime);

private:
    QGraphicsItem *FItem;
    const int FDuration;
};

#endif // OPACITYANIMATION_H
