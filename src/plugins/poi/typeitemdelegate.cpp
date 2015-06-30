#include <QApplication>
#include <QPainter>
#include <definitions/resources.h>
#include <utils/iconstorage.h>
#include "typeitemdelegate.h"

#define  CONNECT_ICON        "connect"
#define  CONNECTEND_ICON     "connectend"
#define  CONNECTLONG_ICON    "connectlong"

void TypeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
// Qt::UserRole contains display data
    QString userRole = index.data(Qt::UserRole).toString();

    QStyleOptionViewItemV4 opt = option;
    initStyleOption(&opt, index);

    QString copyText = opt.text; // the same DisplayRole

    bool flag=false;
    opt.text = "";
    if(userRole.contains("dir:"))
        flag=true;
    else
        opt.rect.setX(16);

// QStandardItem
    const QStyleOptionViewItemV4 *v4 = qstyleoption_cast<const QStyleOptionViewItemV4 *>(&option);
    const QWidget *widget = v4->widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
    painter->save();
    if(!flag)
    {
        QRect backgroundRect(option.rect.x(), option.rect.y(), 18, 18);
        QPixmap pix = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->fileFullName(CONNECT_ICON);
        style->drawItemPixmap(painter,backgroundRect,Qt::AlignLeft,pix);
    }

    QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active))
        cg = QPalette::Inactive;

    QPen pen;
    QPalette palette= opt.palette;
    if (opt.state & QStyle::State_Selected)
    {
        palette.setColor(cg,QPalette::HighlightedText,Qt::white);//color set
        pen.setColor(palette.color(cg,QPalette::HighlightedText));
    }
    else
    {
        palette.setColor(cg, QPalette::Text,Qt::black);//blue
        pen.setColor(palette.color(cg,QPalette::Text));
    }
    painter->setPen(pen);

    QRect rect = opt.rect;
    int shift = 24;
    QFont font;
    rect.setX(rect.x()+shift);
    if(flag)
        font.setBold(true);
    else
        font.setBold(false);

    painter->setFont(font);
    painter->drawText(rect,opt.displayAlignment,copyText);//copyText.toUpper()
    painter->restore();
}

QSize TypeItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QSize result = QStyledItemDelegate::sizeHint(option, index);
    if(result.height()<16)
        result.setHeight(16);
    result.setHeight(result.height()*1.15);
    return result;
}
