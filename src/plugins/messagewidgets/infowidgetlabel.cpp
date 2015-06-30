#include "infowidgetlabel.h"
#include "infowidget.h"

#include <QMouseEvent>

InfoWidgetLabel::InfoWidgetLabel(QWidget *parent) :
    QLabel(parent)
{
}

void InfoWidgetLabel::mousePressEvent(QMouseEvent *ev)
{
    clickPos=ev->pos();
}

void InfoWidgetLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->pos()==clickPos)
    {
        InfoWidget *infoWidget=qobject_cast<InfoWidget *>(parent());
        if (infoWidget)
            emit clicked(infoWidget->contactJid().full(), infoWidget->streamJid().full());
    }
    clickPos=QPoint();
}
