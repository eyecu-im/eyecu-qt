#include "opacityanimation.h"

OpacityAnimation::OpacityAnimation(int ADuration, QObject *parent) :
    QAbstractAnimation(parent),
    FDuration(ADuration)
{}

void OpacityAnimation::updateCurrentTime(int ACurrentTime)
{
    if (FItem)
        FItem->setOpacity((qreal)ACurrentTime/FDuration);
}
